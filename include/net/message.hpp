#pragma once

#include <vector>
#include <string>
#include <memory>

#include "net/common.hpp"

namespace Net
{
    class Message_header
    {
    public:
        MsgTypes id;
        unsigned short size = 0;
    };

    class Message
    {
    public:
        Message_header m_header;
        std::vector<uint8_t> m_body;

    public:
        size_t size() const;

        Message &operator<<(int data);
        Message &operator>>(int &data);
        Message &operator<<(int64_t data);
        Message &operator>>(int64_t &data);
        Message &operator<<(float data);
        Message &operator>>(float &data);
        Message &operator<<(double data);
        Message &operator>>(double &data);
        Message &operator<<(char data);
        Message &operator>>(char &data);
        Message &operator<<(bool data);
        Message &operator>>(bool &data);
        Message &operator<<(const std::string &data);
        Message &operator>>(std::string &data);
    };

    class Connection;

    class Owned_message
    {
    public:
        std::shared_ptr<Connection> remote = nullptr;
        Message msg;
    };

} // namespace Net
