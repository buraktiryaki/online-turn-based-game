#pragma once

#include "game/gameCommon.hpp"
#include "netClient.hpp"

namespace Game
{

class GameClient : public GameCommon
{
public:
    Net::NetClient *m_client;

public:
    GameClient(Net::NetClient *c, int numPlayers = 2,
               int mapSizeX = 8, int mapSizeZ = 3);

    bool readFromMainLoop(Net::Message &msg);

    ~GameClient();
};


}