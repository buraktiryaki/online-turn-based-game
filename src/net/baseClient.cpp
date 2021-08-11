#include "net/baseClient.hpp"

#include <iostream>

namespace Net
{

BaseClient::BaseClient()
    : m_resolver(m_asioContext)
{
}

bool BaseClient::connect(const std::string &host, const uint16_t port)
{
    boost::asio::ip::tcp::resolver::results_type endpoint =
        m_resolver.resolve(host, std::to_string(port));
    m_connection =
        std::make_unique<Connection>(Connection::Owner::client,
                                     m_asioContext,
                                     boost::asio::ip::tcp::socket(m_asioContext),
                                     m_msgIn);
    m_connection->ConnectToServer(endpoint);
    m_threadContext = std::thread([this]() { m_asioContext.run(); });

    //check serverAccept
    //burası olmazsa race var: bağlanıp mesaj gönderirse validation yerine sayılıyor
    auto time = std::chrono::steady_clock::now();
    time += std::chrono::seconds(8); //timeout
    while (std::chrono::steady_clock::now() < time)
    {
        if (!m_connection->IsConnected())
        {
            std::cout << "conn err." << std::endl;
            return false;
        }

        if (!m_msgIn.empty())
        {
            Net::Message msg = m_msgIn.pop_front().msg;
            if (msg.m_header.id == Net::MsgTypes::ServerAccept)
            {
                std::cout << "server accepted the connection" << std::endl;
                onAccepted();
                return true;
            }
            else
            {
                std::cout << "server deny the connection" << std::endl;
                return false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(90));
    }
    std::cout << "connection timed out" << std::endl;
    return false;
}

void BaseClient::disconnect()
{
    if (isConnected())
    {
        m_connection->Disconnect();
    }

    m_asioContext.stop();
    if (m_threadContext.joinable())
        m_threadContext.join();

    m_connection.reset();
}

void BaseClient::onAccepted() {}
void BaseClient::onUpdate() {}
void BaseClient::onMessage(Net::Message &msg) {}

void BaseClient::run(uint8_t fpsLimit)
{
    while (isConnected())
    {
        auto now = std::chrono::steady_clock::now();
        auto nextFrame = now + std::chrono::milliseconds(1000 / fpsLimit);

        runNoLimit();

        std::this_thread::sleep_until(nextFrame);
    }
}

void BaseClient::runNoLimit()
{
    if (isConnected())
    {
        //for continuous incoming messages (can block while)
        uint16_t readCount = m_msgIn.count();
        while (readCount--)
        {
            Net::Message msg = m_msgIn.pop_front().msg;
            onMessage(msg);
        }
        onUpdate();
        //WARN!: onupdate pos is changed
    }
    else
    {
        static bool first = true;
        if (first)
        {
            std::cout << "connection lost" << std::endl;
            first = false;
        }
    }
}

void BaseClient::send(const Message &msg)
{
    //check isConnected (it can cause err)
    m_connection->Send(msg);
}

bool BaseClient::isConnected()
{
    if (m_connection)
        return m_connection->IsConnected();
    else
        return false;
}

BaseClient::~BaseClient()
{
}

};