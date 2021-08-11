#pragma once

#include <cstdint>

namespace Net
{

enum class MsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	//game status
	JoinGame,
	GetReady,
	GameStarted,
	//game data
	GameData,
	PlayerData,
	MapData,
	ShipData,
	//game orders
	GameOrder_BEGIN,
	BasicOrder,
	Rotate,
	Move,
	CallShip,
	Attack,
	EndTurn,
	GameOrder_END,

	//call ship bitti, test et, gui tiklamaliarinda oyunu disable et
	// gui tiklarken hexde seçebiliyor!!!
	//fire order yap

	//MessageAll,
	//ServerMessage,
    //ClientReadytoGame,
	ClientToServer,
	//ServerTime,
	////game
	//ClientGameId
};


} // namespace Net


