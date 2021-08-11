#pragma once

#include <memory>

#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "net/tsqueue.hpp"
#include "net/message.hpp"

//0x0001C0DEC0DEC0DE: first 4 bayt is version num (0001)
#define VALIDATION_KEY 0x0001C0DEC0DEC0DE

namespace Net
{

class BaseServer;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    enum class Owner
    {
        server,
        client
    };

    Connection(Owner owner, boost::asio::io_context &context,
               boost::asio::ip::tcp::socket socket,
               tsqueue<Owned_message> &qIn, BaseServer *ptr = nullptr);

    void ConnectToClient(int uid = -1);
    void ConnectToServer(const boost::asio::ip::tcp::resolver::results_type &endpoints);
    void Disconnect();
    bool IsConnected();
    boost::asio::ip::tcp::socket &getSocket();
    unsigned int getId();
    void Send(const Net::Message &msg);

    ~Connection();

private:
    void WriteValidation();
    void ReadValidation();
    void WriteHeader();
    void WriteBody();
    void ReadHeader();
    void ReadBody();
    void AddToIncomingMessageQueue();

private:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_context &m_asioContext;

    tsqueue<Message> m_qMsgOut;
    tsqueue<Owned_message> &m_qMsgIn;
    Message m_msgTemporaryIn;

    BaseServer *m_serverPtr;
    Owner m_ownerType;
    int m_id;

    //handshake
    uint64_t m_handshakeOut = 0;
    uint64_t m_handshakeIn = 0;
    uint64_t m_handshakeCheck = 0;

public:
    //data
    void *m_data = NULL;
};

};