#pragma once

#include <string>
#include <vector>

#include "net/message.hpp"

#define MAX_PLAYER_COUNT 2
#define MAP_SIZE_X 8
#define MAP_SIZE_Z 8

namespace Game
{

struct PlayerCommon;
struct ShipCommon;
struct MapCellCommon;

enum class SpaceShipType : unsigned int
{
    Main,
    Gen1,
    Gen2,
    Gen3
};

enum class ShipRotation : unsigned int
{
    Down,
    RightDown,
    RightUp,
    Up,
    LeftUp,
    LeftDown
};

enum class GameState : unsigned int
{
    Waiting,
    Ready,
    Started,
    Finished,
    Disconnected
};

struct PlayerCommon
{
public:
    unsigned int id;
    unsigned int team;
    std::string name;
    int money;
    int income;
    int numCells;

    int mainShipX;
    int mainShipZ;
};

struct ShipCommon
{
public:
    int hitpoints;
    int damage;
    ShipRotation rotation;
    SpaceShipType type;

    bool turnMove;
    bool turnRotate;
    bool turnAttack;

    void giveOrderRights();
};

struct MapCellCommon
{
public:
    int indexX;
    int indexZ;
    int ownerId = -1;
    ShipCommon *ship = NULL;

    void newShip(SpaceShipType t, ShipRotation r);
    void delShip();
};

class MapCommon
{
public:
    int sizeX;
    int sizeZ;
    MapCellCommon **cells;

public:
    MapCommon();
    void create(int numX, int numZ);
    MapCellCommon* calculateCellRelatively(int x, int z, ShipRotation r);
    ~MapCommon();
};

class GameCommon
{
public:
    unsigned int m_numPlayers;
    unsigned int m_turn;
    unsigned int m_turnCount;
    uint64_t m_nextTurnTimeEpoch;
    int m_winningPlayerId;
    GameState m_state;
    PlayerCommon *m_players;
    MapCommon m_map;

public:
    GameCommon(int numPlayers = 2, int mapSizeX = 8, int mapSizeZ = 3);

    void setGameData(Net::Message &msg);
    void getGameData(Net::Message &msg);
    void setPlayerData(Net::Message &msg);
    void getPlayerData(Net::Message &msg);
    void setMapData(Net::Message &msg);
    void getMapData(Net::Message &msg);
    void setShipData(Net::Message &msg);
    void getShipData(Net::Message &msg);

    ~GameCommon();
};

}; //namespace Game