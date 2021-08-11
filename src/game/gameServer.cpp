#include "game/gameServer.hpp"
#include "netServer.hpp"

#include <iostream>

using namespace std;
using namespace Game;

GameServer::GameServer(int numPlayers, int mapSizeX, int mapSizeZ)
    : GameCommon(numPlayers, mapSizeX, mapSizeZ)
{
    m_connectedPlayers = 0;
    m_gameStarted = false;

    m_GameCon = new shared_ptr<Net::Connection>[numPlayers];
    for (int i = 0; i < numPlayers; i++)
        m_GameCon[i] = nullptr;
}

void GameServer::clientConnected(std::shared_ptr<Net::Connection> cli)
{
    if (m_connectedPlayers < m_numPlayers)
    {
        Net::ConnectionDataSvr *data = (Net::ConnectionDataSvr *)cli->m_data;
        m_GameCon[data->gameId] = cli;
        m_connectedPlayers++;
        cout << cli->getId() << "(joined game):"
             << data->gameId << "-" << data->name << endl;

        //start game if all players connected
        if (m_connectedPlayers == m_numPlayers)
        {
            m_gameStarted = true;
            startGame();
        }
    }
}

void GameServer::startGame()
{
    m_startTime = chrono::system_clock::now() + chrono::seconds(5);
    m_nextTurnTime = m_startTime + chrono::seconds(TURN_TIME);
    m_nextTurnTimeEpoch = chrono::system_clock::to_time_t(m_nextTurnTime);
    m_turn = 0;
    m_turnCount = 0;

    //set player data
    for (int i = 0; i < m_numPlayers; i++)
    {
        Net::ConnectionDataSvr *data =
            (Net::ConnectionDataSvr *)m_GameCon[i]->m_data;
        m_players[i].id = data->gameId;
        m_players[i].name = data->name;
        m_players[i].team = m_players[i].id;
        m_players[i].numCells = 5;
        m_players[i].income = 5 * 25;
    }

    //give first player money (first turn)
    m_players[0].money = 125;
    m_players[1].money = 0;

    //set players main ship pos
    m_players[0].mainShipX = 0;
    m_players[0].mainShipZ = 1;
    m_players[1].mainShipX = 7;
    m_players[1].mainShipZ = 1;

    //set first hex and ship
    m_map.calculateCellRelatively(0, 1, ShipRotation::Up)->ownerId = 0;
    m_map.calculateCellRelatively(0, 1, ShipRotation::RightUp)->ownerId = 0;
    m_map.calculateCellRelatively(0, 1, ShipRotation::RightDown)->ownerId = 0;
    m_map.calculateCellRelatively(0, 1, ShipRotation::Down)->ownerId = 0;
    m_map.cells[1][0].ownerId = 0;
    m_map.cells[1][0].newShip(SpaceShipType::Main, ShipRotation::RightUp);
    m_map.cells[1][0].ship->giveOrderRights();

    m_map.calculateCellRelatively(7, 1, ShipRotation::Up)->ownerId = 1;
    m_map.calculateCellRelatively(7, 1, ShipRotation::LeftUp)->ownerId = 1;
    m_map.calculateCellRelatively(7, 1, ShipRotation::LeftDown)->ownerId = 1;
    m_map.calculateCellRelatively(7, 1, ShipRotation::Down)->ownerId = 1;
    m_map.cells[1][7].ownerId = 1;
    m_map.cells[1][7].newShip(SpaceShipType::Main, ShipRotation::LeftDown);
    m_map.cells[1][7].ship->giveOrderRights();

    m_state = GameState::Ready;

    Net::Message msg;
    msg.m_header.id = Net::MsgTypes::GetReady;
    sendMessagetoAllPlayers(msg);
    sendGameData();
    sendPlayerData();
    sendMapData();
    sendShipData();

    cout << "[GAME]Get Ready" << endl;
}

void GameServer::gameLoop()
{
    auto now = chrono::system_clock::now();
    if (now < m_startTime)
        return;
    else if (m_state == GameState::Ready)
    {
        m_state = GameState::Started;
        Net::Message msg;
        msg.m_header.id = Net::MsgTypes::GameStarted;
        sendMessagetoAllPlayers(msg);
        cout << "[GAME] Game started " << endl;
    }

    //game started...

    //change turn
    if (now > m_nextTurnTime)
        changeTurn();
}

void GameServer::readFromMainLoop(Net::Message &msg, std::shared_ptr<Net::Connection> con)
{
    if (!m_gameStarted)
        return;

    Net::ConnectionDataSvr *data = (Net::ConnectionDataSvr *)con->m_data;
    int pid = data->gameId;
    if (m_turn != pid)
    {
        cout << "pid:" << pid << " not your turn, order ignored!" << endl;
        return;
    }

    switch (msg.m_header.id)
    {
    case Net::MsgTypes::BasicOrder:
    {
        m_players[pid].money += 25;
        cout << "pid:" << pid << " order_OK" << endl;
        sendPlayerData();
        break;
    }
    case Net::MsgTypes::Rotate:
    {
        int x, z;
        ShipRotation r;
        msg >> (int &)r >> (int &)z >> (int &)x;

        //no ship, no ownership, x and z are wrong, no right to rotate
        bool err = x >= m_map.sizeX || z >= m_map.sizeZ ||
                   x < 0 || z < 0 ||
                   m_map.cells[z][x].ownerId != pid ||
                   m_map.cells[z][x].ship == NULL ||
                   !m_map.cells[z][x].ship->turnRotate;
        if (err)
        {
            cout << "pid:" << pid << " err!" << endl;
            return;
        }

        ShipCommon *ship = m_map.cells[z][x].ship;
        ship->turnRotate = false;
        ship->rotation = r;
        sendShipData();
        break;
    }

    case Net::MsgTypes::Move:
    {
        int x, z;
        ShipRotation r;
        msg >> (int &)r >> (int &)z >> (int &)x;

        //no ship, no ownership, x and z are wrong, no right to move
        bool err = x >= m_map.sizeX || z >= m_map.sizeZ ||
                   x < 0 || z < 0 ||
                   m_map.cells[z][x].ownerId != pid ||
                   m_map.cells[z][x].ship == NULL ||
                   !m_map.cells[z][x].ship->turnMove;
        if (err)
        {
            cout << "pid:" << pid << " err!" << endl;
            return;
        }

        MapCellCommon *oldCell = &(m_map.cells[z][x]);
        MapCellCommon *newCell = m_map.calculateCellRelatively(x, z, r);

        //if newCell == null accessing newCell->ship is dangerous
        if (newCell == NULL || newCell->ship != NULL)
        {
            cout << "pid:" << pid << " direction-err!" << endl;
            return;
        }

        if (oldCell->ship->rotation != r && oldCell->ship->turnRotate == false)
        {
            cout << "pid:" << pid << " no right to rotate!" << endl;
            return;
        }

        //set rights
        if (oldCell->ship->rotation == r)
        {
            oldCell->ship->turnMove = false;
        }
        else
        {
            oldCell->ship->turnMove = false;
            oldCell->ship->turnRotate = false;
        }

        // opponent's hex is taken
        if (newCell->ownerId != -1 && newCell->ownerId != pid)
        {
            m_players[newCell->ownerId].numCells--;
            m_players[newCell->ownerId].income -= 25;
            m_players[pid].numCells++;
            m_players[pid].income += 25;
        }
        //a empty hex is taken
        else if (newCell->ownerId == -1)
        {
            m_players[pid].numCells++;
            m_players[pid].income += 25;
        }

        //if the ship is main ship change its pos
        if (oldCell->ship->type == SpaceShipType::Main)
        {
            m_players[pid].mainShipX = newCell->indexX;
            m_players[pid].mainShipZ = newCell->indexZ;
        }

        //change ship pos
        newCell->ship = oldCell->ship;
        newCell->ship->rotation = r;
        oldCell->ship = NULL;
        newCell->ownerId = pid;

        sendShipData();
        sendMapData();
        sendPlayerData();
        break;
    }

    case Net::MsgTypes::CallShip:
    {
        SpaceShipType type;
        ShipRotation rot;
        msg >> (int &)rot >> (int &)type;

        int mainX = m_players[pid].mainShipX;
        int mainZ = m_players[pid].mainShipZ;
        MapCellCommon *mainCell = &(m_map.cells[mainZ][mainX]);
        MapCellCommon *newCell = m_map.calculateCellRelatively(mainX, mainZ, rot);

        //wrong direction or not empty or no ownership
        if (!newCell || newCell->ship != NULL || newCell->ownerId != pid)
        {
            cout << "err-line:" << __LINE__ << endl;
            return;
        }

        int moneyNeeded = 0;
        if (type == SpaceShipType::Gen1)
            moneyNeeded = 100;
        else if (type == SpaceShipType::Gen2)
            moneyNeeded = 200;
        else if (type == SpaceShipType::Gen3)
            moneyNeeded = 600;

        //not enough money
        if (m_players[pid].money < moneyNeeded)
        {
            cout << "err-line:" << __LINE__ << endl;
            return;
        }

        //all conditions are ok
        //create ship---
        m_players[pid].money -= moneyNeeded;
        newCell->newShip(type, rot);
        newCell->ship->giveOrderRights();
        sendShipData();
        sendPlayerData();
        break;
    }

    case Net::MsgTypes::Attack:
    {
        int x, z;
        ShipRotation r;
        msg >> (int &)r >> (int &)z >> (int &)x;

        bool err = x >= m_map.sizeX || z >= m_map.sizeZ ||
                   x < 0 || z < 0 ||
                   m_map.cells[z][x].ownerId != pid ||
                   m_map.cells[z][x].ship == NULL ||
                   !m_map.cells[z][x].ship->turnAttack;
        if (err)
        {
            cout << "err-line:" << __LINE__ << endl;
            return;
        }

        MapCellCommon *currentCell = &(m_map.cells[z][x]);
        MapCellCommon *targetCell = m_map.calculateCellRelatively(x, z, r);

        //targetcell err
        if (!targetCell || targetCell->ownerId == -1 ||
            targetCell->ownerId == pid || targetCell->ship == NULL)
        {
            cout << "err-line:" << __LINE__ << endl;
            return;
        }

        //direction not same
        if (r != currentCell->ship->rotation)
        {
            //no right to rotate.
            if (currentCell->ship->turnRotate == false)
            {
                cout << "err-line:" << __LINE__ << endl;
                return;
            }
            currentCell->ship->rotation = r;
            currentCell->ship->turnRotate = false;
        }
        currentCell->ship->turnAttack = false;

        //all conditions are ok
        //attack---

        targetCell->ship->hitpoints -= currentCell->ship->damage;
        if (targetCell->ship->hitpoints <= 0)
        {
            if (targetCell->ship->type == SpaceShipType::Main)
                finishGame(currentCell->ownerId);
            targetCell->delShip();
            sendShipData();
            return;
        }

        bool enemyLookHere = abs((int)targetCell->ship->rotation - (int)currentCell->ship->rotation) == 3;
        if (enemyLookHere)
        {
            currentCell->ship->hitpoints -= targetCell->ship->damage;
            if (currentCell->ship->hitpoints <= 0)
            {
                if (currentCell->ship->type == SpaceShipType::Main)
                    finishGame(targetCell->ownerId);
                currentCell->delShip();
            }
        }
        sendShipData();
        break;
    }

    case Net::MsgTypes::EndTurn:
    {
        m_nextTurnTime = chrono::system_clock::now();
        changeTurn();
        break;
    }

    default:
    {
        break;
    }
    };
}

void GameServer::sendGameData()
{
    //send game data
    Net::Message msg;
    setGameData(msg);
    sendMessagetoAllPlayers(msg);
}

void GameServer::sendPlayerData()
{
    //send player data
    Net::Message msg;
    setPlayerData(msg);
    sendMessagetoAllPlayers(msg);
}

void GameServer::sendMapData()
{
    Net::Message msg;
    setMapData(msg);
    sendMessagetoAllPlayers(msg);
}

void GameServer::sendShipData()
{
    Net::Message msg;
    setShipData(msg);
    sendMessagetoAllPlayers(msg);
}

void GameServer::sendMessagetoAllPlayers(Net::Message &msg)
{
    for (int i = 0; i < m_numPlayers; i++)
    {
        if (m_GameCon[i])
            m_GameCon[i]->Send(msg);
    }
}

void GameServer::changeTurn()
{
    m_nextTurnTime = m_nextTurnTime + chrono::seconds(TURN_TIME);
    m_nextTurnTimeEpoch = chrono::system_clock::to_time_t(m_nextTurnTime);

    m_turnCount++;
    m_turn = m_turnCount % m_numPlayers;

    //give money
    m_players[m_turn].money += m_players[m_turn].income;

    //clear ship rights
    clearAllShipRights();

    sendGameData();
    sendPlayerData();
    sendMapData();
    sendShipData();
}

void GameServer::clearAllShipRights()
{
    for (int z = 0; z < m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_map.sizeX; x++)
        {
            ShipCommon *s = m_map.cells[z][x].ship;
            if (s)
            {
                s->giveOrderRights();
            }
        }
    }
}

void GameServer::finishGame(int winnigPlayerId)
{
    m_winningPlayerId = winnigPlayerId;
    m_state = GameState::Finished;
    sendGameData();
}

GameServer::~GameServer()
{
}