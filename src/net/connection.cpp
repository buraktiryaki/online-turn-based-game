#include <iostream>

#include "net/connection.hpp"
#include "net/baseServer.hpp"

Net::Connection::Connection(Owner owner, boost::asio::io_context &context,
                            boost::asio::ip::tcp::socket socket,
                            Net::tsqueue<Net::Owned_message> &qIn,
                            Net::BaseServer *ptr)
    : m_asioContext(context), m_socket(std::move(socket)),
      m_qMsgIn(qIn), m_ownerType(owner), m_serverPtr(ptr)
{
    //inits
    m_ownerType = owner;
    m_serverPtr = ptr;
    m_id = -1;

    if (m_ownerType == Owner::server)
    {
        //steady_clock: (client can't estimate)
        m_handshakeOut = std::chrono::steady_clock::now().time_since_epoch().count();
        m_handshakeCheck = m_handshakeOut ^ VALIDATION_KEY;
    }
}

void Net::Connection::ConnectToClient(int uid)
{
    if (m_ownerType == Owner::server)
    {
        if (m_socket.is_open())
        {
            m_id = uid;
            WriteValidation();
        }
    }
}

void Net::Connection::ConnectToServer(
    const boost::asio::ip::tcp::resolver::results_type &endpoints)
{
    if (m_ownerType == Owner::client)
    {
        boost::asio::async_connect(m_socket, endpoints,
                                   [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint) {
                                       if (!ec)
                                       {
                                           ReadValidation();
                                       }
                                       else
                                       {
                                           std::cout << ec.message() << std::endl;
                                           m_socket.close();
                                       }
                                   });
    }
}
void Net::Connection::Disconnect()
{
    if (IsConnected())
        boost::asio::post(m_asioContext, [this]() { m_socket.close(); });
}

bool Net::Connection::IsConnected()
{
    return m_socket.is_open();
}

boost::asio::ip::tcp::socket &Net::Connection::getSocket()
{
    return m_socket;
}

unsigned int Net::Connection::getId()
{
    return m_id;
}

void Net::Connection::Send(const Net::Message &msg)
{
    boost::asio::post(m_asioContext,
                      [lifetime = this->shared_from_this(), this, msg]() {
                          bool bWritingMessage = !(m_qMsgOut.empty());
                          m_qMsgOut.push_back(msg);
                          if (!bWritingMessage)
                          {
                              WriteHeader();
                          }
                      });
}

void Net::Connection::WriteValidation()
{
    boost::asio::async_write(m_socket, boost::asio::buffer(&m_handshakeOut, sizeof(uint64_t)),
                             [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                 if (!ec)
                                 {
                                     if (m_ownerType == Owner::client)
                                         ReadHeader();
                                     else //server
                                         ReadValidation();
                                 }
                                 else
                                 {
                                     std::cout << "[" << m_id << "] Write Validation Fail.\n";
                                     m_socket.close();
                                 }
                             });
}

void Net::Connection::ReadValidation()
{
    boost::asio::async_read(m_socket, boost::asio::buffer(&m_handshakeIn, sizeof(uint64_t)),
                            [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    if (m_ownerType == Owner::client)
                                    {
                                        m_handshakeOut = m_handshakeIn ^ VALIDATION_KEY;
                                        WriteValidation();
                                    }
                                    else
                                    {
                                        if (m_handshakeIn == m_handshakeCheck)
                                        {
                                            m_serverPtr->onClientValidated(shared_from_this());
                                            ReadHeader();
                                        }
                                        else
                                        {
                                            std::cout << "Client validation failed." << std::endl;
                                            m_socket.close();
                                        }
                                    }
                                }
                                else
                                {
                                    std::cout << "[" << m_id << "] Read validation Fail.\n";
                                    m_socket.close();
                                }
                            });
}

void Net::Connection::WriteHeader()
{
    boost::asio::async_write(m_socket, boost::asio::buffer(&m_qMsgOut.front().m_header, sizeof(Message::m_header)),
                             [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                 if (!ec)
                                 {
                                     if (m_qMsgOut.front().m_body.size() > 0)
                                     {
                                         WriteBody();
                                     }
                                     else
                                     {
                                         m_qMsgOut.pop_front();

                                         if (!m_qMsgOut.empty())
                                         {
                                             WriteHeader();
                                         }
                                     }
                                 }
                                 else
                                 {
                                     std::cout << "[" << m_id << "] Write Header Fail.\n";
                                     m_socket.close();
                                 }
                             });
}

void Net::Connection::WriteBody()
{
    boost::asio::async_write(m_socket, boost::asio::buffer(m_qMsgOut.front().m_body.data(), m_qMsgOut.front().m_body.size()),
                             [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                 if (!ec)
                                 {
                                     m_qMsgOut.pop_front();
                                     if (!m_qMsgOut.empty())
                                     {
                                         WriteHeader();
                                     }
                                 }
                                 else
                                 {
                                     std::cout << "[" << m_id << "] Write Body Fail.\n";
                                     m_socket.close();
                                 }
                             });
}

void Net::Connection::ReadHeader()
{
    boost::asio::async_read(m_socket, boost::asio::buffer(&m_msgTemporaryIn.m_header, sizeof(Message::m_header)),
                            [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    if (m_msgTemporaryIn.m_header.size > 0)
                                    {
                                        m_msgTemporaryIn.m_body.resize(m_msgTemporaryIn.m_header.size);
                                        ReadBody();
                                    }
                                    else
                                    {
                                        AddToIncomingMessageQueue();
                                    }
                                }
                                else
                                {
                                    std::cout << "[" << m_id << "] Read Header Fail.\n";
                                    m_socket.close();
                                }
                            });
}

void Net::Connection::ReadBody()
{
    boost::asio::async_read(m_socket, boost::asio::buffer(m_msgTemporaryIn.m_body.data(), m_msgTemporaryIn.m_body.size()),
                            [lifetime = this->shared_from_this(), this](std::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    AddToIncomingMessageQueue();
                                }
                                else
                                {
                                    std::cout << "[" << m_id << "] Read Body Fail.\n";
                                    m_socket.close();
                                }
                            });
}

void Net::Connection::AddToIncomingMessageQueue()
{
    if (m_ownerType == Owner::server)
        m_qMsgIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
    else
        m_qMsgIn.push_back({nullptr, m_msgTemporaryIn});

    ReadHeader();
}

Net::Connection::~Connection()
{
    std::cout << "connnection is destructed-" << m_id << std::endl;
}