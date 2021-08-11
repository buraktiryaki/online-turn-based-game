#include "net/baseServer.hpp"

#include <iostream>
#include <chrono>

#include <boost/bind/bind.hpp>

namespace Net
{

BaseServer::BaseServer(unsigned short port)
    : m_idCounter(100),
      m_asioAcceptor(m_asioContext,
                     boost::asio::ip::tcp::endpoint(
                         boost::asio::ip::tcp::v4(), port))
{
    m_stopMainLoop = false;
}

void BaseServer::startAccept()
{
    boost::asio::ip::tcp::socket socket(m_asioContext);
    std::shared_ptr<Connection> con =
        std::make_shared<Connection>(Connection::Owner::server, m_asioContext,
                                     std::move(socket), m_msgIn, this);

    m_asioAcceptor.async_accept(con->getSocket(),
                                boost::bind(&BaseServer::handleAccept, this,
                                            con,
                                            boost::asio::placeholders::error));
}

void BaseServer::handleAccept(std::shared_ptr<Connection> con,
                              const boost::system::error_code &error)
{
    if (!error)
    {
        if (onClientConnect(con))
        {
            std::cout << "New Connection: "
                      << con->getSocket().remote_endpoint() << std::endl;
            con->ConnectToClient(m_idCounter++);
            m_connections.push_back(std::move(con));
        }
        else
        {
            std::cout << "Connection refused" << std::endl;
        }
    }
    else
    {
        std::cout << error.message() << std::endl;
    }
    startAccept();
}

void BaseServer::clearDisconnectedClients()
{
    bool disconnectedExist = false;

    for (auto &client : m_connections)
    {
        if (!client->IsConnected())
        {
            onClientDisconnect(client);
            client.reset();
            disconnectedExist = true;
        }
    }
    if (disconnectedExist)
    {
        m_connections.erase(std::remove(m_connections.begin(),
                                        m_connections.end(), nullptr),
                            m_connections.end());
    }
}

bool BaseServer::onClientConnect(std::shared_ptr<Net::Connection> con)
{
    return false;
}

void BaseServer::onClientValidated(std::shared_ptr<Net::Connection> con) {}
void BaseServer::onUpdate() {}
void BaseServer::onMessage(std::shared_ptr<Net::Connection> con,
                           Net::Message &msg) {}
void BaseServer::onClientDisconnect(std::shared_ptr<Net::Connection> con) {}

void BaseServer::start()
{
    startAccept();
    m_threadContext = std::thread([this]() {
        m_asioContext.run();
    });
}

void BaseServer::stop()
{
    m_asioContext.stop();
    if (m_threadContext.joinable())
    {
        m_threadContext.join();
    }
}

void BaseServer::run(uint8_t fpsLimit)
{
    for (;!m_stopMainLoop;)
    {
        auto now = std::chrono::steady_clock::now();
        auto nextFrame = now + std::chrono::milliseconds(1000 / fpsLimit);

        clearDisconnectedClients();

        //for continuous incoming messages (it can block while loop)
        uint16_t readCount = m_msgIn.count();
        while (readCount--)
        {
            Net::Owned_message msg = m_msgIn.pop_front();
            onMessage(msg.remote, msg.msg);
        }
        onUpdate();
        //WARN! : onUpdate pos is changed

        std::this_thread::sleep_until(nextFrame);
    }
}

BaseServer::~BaseServer()
{
    stop();
    std::cout << "~BaseServer()" << std::endl;
}

};