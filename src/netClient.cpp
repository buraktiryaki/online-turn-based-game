#include <iostream>

#include "netClient.hpp"
#include "mainClient.hpp"

using namespace std;
using namespace Net;

NetClient::NetClient(MainClient *c)
{
    m_mainClient = c;
}

void NetClient::onAccepted()
{
    Net::Message msg;
    msg.m_header.id = Net::MsgTypes::JoinGame;
    msg << (int)m_mainClient->m_gameId
        << m_mainClient->m_gameUname;
    send(msg);
}

void NetClient::onUpdate()
{
    static auto lastTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto deltaTime = std::chrono::duration<float, std::milli>(now - lastTime);
    lastTime = now;
    //cout << deltaTime.count() << endl;
}

void NetClient::onMessage(Net::Message &msg)
{
    std::cout << "msgid:" << (uint32_t)msg.m_header.id << std::endl;
    if (msg.m_header.id == Net::MsgTypes::ServerAccept)
    {
        msg.m_header.id = Net::MsgTypes::ClientToServer;
        msg << "I'm a client.";
        send(msg);
    }
    else if (msg.m_header.id == Net::MsgTypes::ServerPing)
    {
        uint64_t msgTime;
        msg >> (int64_t &)msgTime;
        auto timeOld = std::chrono::steady_clock::duration(msgTime);
        auto timeNow = chrono::steady_clock::now().time_since_epoch();

        cout << "ping: "
             << chrono::duration_cast<chrono::milliseconds>(timeNow - timeOld).count()
             << endl;
    }

    // game related messages
    m_mainClient->m_game.readFromMainLoop(msg);
}

void NetClient::sendPing()
{
    Net::Message msg;
    auto now = chrono::steady_clock::now().time_since_epoch();
    msg << now.count();
    msg.m_header.id = Net::MsgTypes::ServerPing;
    send(msg);
}

std::shared_ptr<Net::Connection> NetClient::getConnection()
{
    return m_connection;
}

NetClient::~NetClient()
{
}
