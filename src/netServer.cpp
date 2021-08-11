#include <iostream>

#include "netServer.hpp"

using namespace std;
using namespace Net;

NetServer::NetServer(uint16_t port)
    : Net::BaseServer(port)
{
}

bool NetServer::onClientConnect(std::shared_ptr<Net::Connection> con)
{
    return true;
    //return false; //deny client
}

void NetServer::onClientValidated(std::shared_ptr<Net::Connection> con)
{
    con->m_data = new ConnectionDataSvr;
    Net::Message msg;
    msg.m_header.id = Net::MsgTypes::ServerAccept;
    con->Send(msg);
    cout << con->getId() << " Validated" << endl;
}

void NetServer::onUpdate()
{
    static auto lastTime = chrono::steady_clock::now();
    auto now = chrono::steady_clock::now();
    auto deltaTime = chrono::duration<float, std::milli>(now - lastTime);
    lastTime = now;
    //cout << deltaTime.count() << endl;

    if (m_game.m_gameStarted)
    {
        m_game.gameLoop();
    }
    if (m_game.m_state == Game::GameState::Finished)
    {
        static chrono::steady_clock::time_point closeTime =
            chrono::steady_clock::now() + chrono::seconds(3);
        static bool first = true;
        if (first)
        {
            cout << "server will shut down in 3 seconds" << endl;
            first = false;
        }
        if (chrono::steady_clock::now() > closeTime)
            m_stopMainLoop = true;
    }
}

void NetServer::onMessage(std::shared_ptr<Net::Connection> con,
                          Net::Message &msg)
{
    cout << "[MESSAGE]"
         << "[" << con->getSocket().remote_endpoint() << "] "
         << (int)msg.m_header.id << endl;
    if (msg.m_header.id == Net::MsgTypes::ClientToServer)
    {
        string str;
        msg >> str;
        std::cout << "[" << con->getId() << "] "
                  << (int)msg.m_header.id << "=";
        cout << str << endl;
    }
    else if (msg.m_header.id == Net::MsgTypes::ServerPing)
    {
        cout << "(ping) " << con->getId() << endl;
        con->Send(msg);
    }
    else if (msg.m_header.id == Net::MsgTypes::JoinGame)
    {
        ConnectionDataSvr *data = (ConnectionDataSvr *)con->m_data;
        msg >> data->name;
        msg >> (int &)data->gameId;

        if (data->gameId >= m_game.m_numPlayers ||
            m_game.m_GameCon[data->gameId] != nullptr)
        {
            con->Disconnect();
            cout << "warn!" << endl;
            cout << data->gameId << endl;
        }
        else
        {
            data->isConnectedToGame = true;
            m_game.clientConnected(con);
        }
    }
    else if ((int)msg.m_header.id > (int)MsgTypes::GameOrder_BEGIN &&
             (int)msg.m_header.id < (int)MsgTypes::GameOrder_END)
    {
        //game related messages
        m_game.readFromMainLoop(msg, con);
    }
    else
    {
        cout << "(unknown package)" << con->getSocket().remote_endpoint() << ": "
             << (int)msg.m_header.id << endl;
    }
}

void NetServer::onClientDisconnect(std::shared_ptr<Net::Connection> con)
{
    ConnectionDataSvr *data = (ConnectionDataSvr *)con->m_data;
    if (data->isConnectedToGame)
    {
        m_game.m_GameCon[data->gameId] = nullptr;
        m_game.m_connectedPlayers--;
        //end the game
        if (m_game.m_gameStarted)
        {
            if (data->gameId == 0)
                m_game.m_winningPlayerId = 1;
            else
                m_game.m_winningPlayerId = 0;
            m_game.m_state = Game::GameState::Finished;
            m_game.sendGameData();
            cout << "Client left the game" << endl;
        }
        m_game.m_gameStarted = false;
    }
    delete data;
    cout << "[" << con->getId() << "] "
         << "Client Disconnected." << endl;
}

NetServer::~NetServer()
{
}