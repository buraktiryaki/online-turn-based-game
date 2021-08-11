#pragma once

#include "net/baseServer.hpp"

#include "game/gameServer.hpp"

namespace Net{

struct ConnectionDataSvr
{
    std::string name;
    unsigned int gameId;
    bool isConnectedToGame;
};

class NetServer : public Net::BaseServer
{
public:
    Game::GameServer m_game;
public:
    NetServer(uint16_t port);

    bool onClientConnect(std::shared_ptr<Net::Connection> con);
    void onClientValidated(std::shared_ptr<Net::Connection> con);
    void onUpdate();
    void onMessage(std::shared_ptr<Net::Connection> con,
                   Net::Message &msg);
    void onClientDisconnect(std::shared_ptr<Net::Connection> con);

    ~NetServer();
};

}//namespace Net
