#pragma once

#include "net/baseClient.hpp"

class MainClient;

namespace Net{


class NetClient : public Net::BaseClient
{
public:
    MainClient *m_mainClient;

public:
    NetClient(MainClient *c);

    void onAccepted();
    void onUpdate();
    void onMessage(Net::Message &msg);

    void sendPing();

    std::shared_ptr<Net::Connection> getConnection();

    ~NetClient();
};

}//namespace Net

