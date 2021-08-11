#include "game/gameCommon.hpp"

using namespace Game;

// __ShipCommon__
void ShipCommon::giveOrderRights()
{
    turnAttack = true;
    turnMove = true;
    turnRotate = true;
}

// __MapCellComon__
void MapCellCommon::newShip(SpaceShipType t, ShipRotation r)
{
    if (this->ship == NULL)
        delShip();
    this->ship = new ShipCommon;

    if (t == SpaceShipType::Main)
    {
        this->ship->hitpoints = 500;
        this->ship->damage = 200;
    }
    else if (t == SpaceShipType::Gen1)
    {
        this->ship->hitpoints = 100;
        this->ship->damage = 50;
    }
    else if (t == SpaceShipType::Gen2)
    {
        this->ship->hitpoints = 200;
        this->ship->damage = 100;
    }
    else if (t == SpaceShipType::Gen3)
    {
        this->ship->hitpoints = 300;
        this->ship->damage = 200;
    }
    this->ship->type = t;
    this->ship->rotation = r;
}

void MapCellCommon::delShip()
{
    delete this->ship;
    this->ship = NULL;
}

// ___MapCommon___
MapCommon::MapCommon()
{
}

void MapCommon::create(int numX, int numZ)
{
    sizeX = numX;
    sizeZ = numZ;

    cells = new MapCellCommon *[numZ];
    for (int z = 0; z < numZ; z++)
    {
        cells[z] = new MapCellCommon[numX];

        //assign x,y
        for (int x = 0; x < numX; x++)
        {
            cells[z][x].indexX = x;
            cells[z][x].indexZ = z;
        }
    }
}

MapCellCommon *MapCommon::calculateCellRelatively(int x, int z, ShipRotation r)
{
    int nextX, nextZ;
    switch (r)
    {
    case ShipRotation::Down:
        nextX = x;
        nextZ = z + 1;
        break;
    case ShipRotation::RightDown:
        nextX = x + 1;
        nextZ = z + (x % 2);
        break;
    case ShipRotation::RightUp:
        nextX = x + 1;
        nextZ = (x % 2) ? z : z - 1;
        break;
    case ShipRotation::Up:
        nextX = x;
        nextZ = z - 1;
        break;
    case ShipRotation::LeftUp:
        nextX = x - 1;
        nextZ = (x % 2) ? z : z - 1;
        break;
    case ShipRotation::LeftDown:
        nextX = x - 1;
        nextZ = z + (x % 2);
        break;
    }

    if (nextX >= sizeX || nextZ >= sizeZ || nextX < 0 || nextZ < 0)
        return NULL;
    else
        return &(cells[nextZ][nextX]);
}

MapCommon::~MapCommon()
{
    for (int z = 0; z < sizeZ; z++)
    {
        delete[] cells[z];
    }
    delete[] cells;
}

// ___GameCommon___
GameCommon::GameCommon(int numPlayers, int mapSizeX, int mapSizeZ)
{
    this->m_numPlayers = numPlayers;
    m_map.create(mapSizeX, mapSizeZ);
    m_players = new PlayerCommon[numPlayers];

    m_state = GameState::Waiting;
    m_turn = 0;
    m_turnCount = 0;
    m_winningPlayerId = -1;
}

void GameCommon::setGameData(Net::Message &msg)
{
    //Packet Structure:
    //(int)numPlayers|(int)turn|(int)turnCount|(int)Gamestate
    msg.m_header.id = Net::MsgTypes::GameData;
    msg << (int)m_numPlayers << (int)m_turn
        << (int)m_turnCount << (int)m_state << (int)m_winningPlayerId
        << (int64_t)m_nextTurnTimeEpoch;
}

void GameCommon::getGameData(Net::Message &msg)
{
    msg >> (int64_t &)m_nextTurnTimeEpoch >> (int &)m_winningPlayerId >>
        (int &)m_state >> (int &)m_turnCount >>
        (int &)m_turn >> (int &)m_numPlayers;
}

void GameCommon::setPlayerData(Net::Message &msg)
{
    //Packet Structure:
    //player1|player2|...|(int)numPlayers|
    //|player| = (int)id|(int)team|(string)name|
    //              (int)money|(int)income|(int)numCells
    msg.m_header.id = Net::MsgTypes::PlayerData;
    for (int i = 0; i < m_numPlayers; i++)
    {
        msg << (int)m_players[i].id << (int)m_players[i].team
            << m_players[i].name << (int)m_players[i].money
            << (int)m_players[i].income << (int)m_players[i].numCells;
    }
    msg << (int)m_numPlayers;
}

void GameCommon::getPlayerData(Net::Message &msg)
{
    msg >> (int &)m_numPlayers;
    for (int i = 0; i < m_numPlayers; i++)
    {
        msg >> (int &)m_players[m_numPlayers - 1 - i].numCells >>
            (int &)m_players[m_numPlayers - 1 - i].income >>
            (int &)m_players[m_numPlayers - 1 - i].money >> m_players[m_numPlayers - 1 - i].name >>
            (int &)m_players[m_numPlayers - 1 - i].team >> (int &)m_players[m_numPlayers - 1 - i].id;
    }
}

void GameCommon::setMapData(Net::Message &msg)
{
    //Packet Structure:
    //|cell|cell|...|mapSizeX|mapSizeZ|
    //cell= |indexX|indexZ|ownerId|
    msg.m_header.id = Net::MsgTypes::MapData;
    for (int z = 0; z < m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_map.sizeX; x++)
        {
            msg << (int)m_map.cells[z][x].ownerId;
        }
    }
    msg << m_map.sizeX << m_map.sizeZ;
}

void GameCommon::getMapData(Net::Message &msg)
{
    msg >> m_map.sizeZ >> m_map.sizeX;
    for (int z = 0; z < m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_map.sizeX; x++)
        {
            // read from back
            msg >> (int &)m_map.cells[m_map.sizeZ - 1 - z][m_map.sizeX - 1 - x].ownerId;
        }
    }
}

void GameCommon::setShipData(Net::Message &msg)
{
    //Packet Structure:
    //ship|ship|...|count|
    //ship = |hitpoints|damage|rotation|type|posX|posZ|tAttack|tMove|tRotate
    msg.m_header.id = Net::MsgTypes::ShipData;
    int count = 0;
    for (int z = 0; z < m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_map.sizeX; x++)
        {
            //there is a ship
            if (m_map.cells[z][x].ship)
            {
                msg << (int)m_map.cells[z][x].ship->hitpoints
                    << (int)m_map.cells[z][x].ship->damage
                    << (int)m_map.cells[z][x].ship->rotation
                    << (int)m_map.cells[z][x].ship->type
                    << (bool)m_map.cells[z][x].ship->turnAttack
                    << (bool)m_map.cells[z][x].ship->turnMove
                    << (bool)m_map.cells[z][x].ship->turnRotate << (int)x << (int)z;
                count++;
            }
        }
    }
    msg << count;
}

void GameCommon::getShipData(Net::Message &msg)
{
    //clear ship data
    for (int z = 0; z < m_map.sizeZ; z++)
    {
        for (int x = 0; x < m_map.sizeX; x++)
        {
            if (m_map.cells[z][x].ship)
            {
                delete m_map.cells[z][x].ship;
                m_map.cells[z][x].ship = NULL;
            }
        }
    }

    int count;
    msg >> count;
    for (int i = 0; i < count; i++)
    {
        int x, z;
        msg >> z >> x;
        m_map.cells[z][x].ship = new ShipCommon;
        msg >> (bool &)m_map.cells[z][x].ship->turnRotate >>
            (bool &)m_map.cells[z][x].ship->turnMove >>
            (bool &)m_map.cells[z][x].ship->turnAttack >>
            (int &)m_map.cells[z][x].ship->type >>
            (int &)m_map.cells[z][x].ship->rotation >>
            (int &)m_map.cells[z][x].ship->damage >>
            (int &)m_map.cells[z][x].ship->hitpoints;
    }
}

GameCommon::~GameCommon()
{
    delete[] m_players;
}
