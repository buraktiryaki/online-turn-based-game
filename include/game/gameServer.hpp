#pragma once

#include "game/gameCommon.hpp"
#include "net/connection.hpp"

#define TURN_TIME 101

namespace Game
{

class GameServer : public GameCommon
{
public:
    int m_connectedPlayers;
    bool m_gameStarted;
    std::chrono::system_clock::time_point m_startTime;
    std::chrono::system_clock::time_point m_nextTurnTime;
    std::shared_ptr<Net::Connection> *m_GameCon;

public:
    GameServer(int numPlayers = 2, int mapSizeX = 8, int mapSizeZ = 3);

    void clientConnected(std::shared_ptr<Net::Connection> cli);
    void startGame();
    void gameLoop();
    void readFromMainLoop(Net::Message &msg, std::shared_ptr<Net::Connection> con);

    void sendGameData();
    void sendPlayerData();
    void sendMapData();
    void sendShipData();
    void sendMessagetoAllPlayers(Net::Message &msg);

    void changeTurn();
    void clearAllShipRights();
    void finishGame(int winnigPlayerId);

    ~GameServer();
};


}; //namespace Game



