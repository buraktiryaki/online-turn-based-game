#pragma once

#include <vector>

#include "net/tsqueue.hpp"
#include "net/message.hpp"
#include "net/connection.hpp"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 30000

namespace Net
{

class BaseClient
{
private:
    boost::asio::io_context m_asioContext;
    boost::asio::ip::tcp::resolver m_resolver;
    std::thread m_threadContext;

protected:
    std::shared_ptr<Connection> m_connection;
    tsqueue<Owned_message> m_msgIn;

public:
    BaseClient();

    bool connect(const std::string &host, const uint16_t port);
    void disconnect();
    bool isConnected();

    virtual void onAccepted();
    virtual void onUpdate();
    virtual void onMessage(Net::Message &msg);

    void run(uint8_t fpsLimit = 90);
    void runNoLimit();
    void send(const Message &msg);

    ~BaseClient();

};//class baseClient


};// namespace Net