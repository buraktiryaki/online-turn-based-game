#include "game/gameClient.hpp"

#include <iostream>

using namespace std;
using namespace Game;

GameClient::GameClient(Net::NetClient *c,
                       int numPlayers, int mapSizeX, int mapSizeZ)
    : GameCommon(numPlayers, mapSizeX, mapSizeZ)
{
    m_client = c;
}

bool GameClient::readFromMainLoop(Net::Message &msg)
{
    switch (msg.m_header.id)
    {
    case Net::MsgTypes::GetReady:
    {
        m_state = Game::GameState::Ready;
        break;
    }
    case Net::MsgTypes::GameStarted:
    {
        m_state = Game::GameState::Started;
        break;
    }
    case Net::MsgTypes::GameData:
    {
        getGameData(msg);
        break;
    }
    case Net::MsgTypes::PlayerData:
    {
        getPlayerData(msg);
        break;
    }
    case Net::MsgTypes::MapData:
    {
        getMapData(msg);
        break;
    }
    case Net::MsgTypes::ShipData:
    {
        getShipData(msg);
        break;
    }

    default:
    {
        break;
    }
    }
    return false;
}

GameClient::~GameClient()
{
}