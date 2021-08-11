#pragma once

#include <vector>

#include "net/tsqueue.hpp"
#include "net/message.hpp"
#include "net/connection.hpp"

namespace Net
{

class BaseServer
{
private:
    boost::asio::io_context m_asioContext;
    boost::asio::ip::tcp::acceptor m_asioAcceptor;
    std::thread m_threadContext;

protected:
    bool m_stopMainLoop;
    unsigned int m_idCounter;
    std::vector<std::shared_ptr<Connection>> m_connections;
    tsqueue<Net::Owned_message> m_msgIn;

    // -fuctions
private:
    void startAccept();
    void handleAccept(std::shared_ptr<Net::Connection> con,
                      const boost::system::error_code &error);
    void clearDisconnectedClients();

protected:
    virtual bool onClientConnect(std::shared_ptr<Net::Connection> con);
    virtual void onUpdate();
    virtual void onMessage(std::shared_ptr<Net::Connection> con,
                           Net::Message &msg);
    virtual void onClientDisconnect(std::shared_ptr<Net::Connection> con);
public:
    virtual void onClientValidated(std::shared_ptr<Net::Connection> con);

public:
    BaseServer(unsigned short port);

    void start();
    void stop();
    void run(uint8_t fpsLimit = 90);

    ~BaseServer();

}; //class baseServer

}; // namespace Net