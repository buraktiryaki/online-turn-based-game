#include "net/message.hpp"

#include <cstring>

size_t Net::Message::size() const
{
    return m_body.size();
}

//write int
Net::Message &Net::Message::operator<<(int data)
{
    size_t i = m_body.size();
    m_body.resize(i + sizeof(int));
    std::memcpy(m_body.data() + i, &data, sizeof(int));
    m_header.size = i + sizeof(int);
    return *this;
}

//read int
Net::Message &Net::Message::operator>>(int &data)
{
    size_t i = m_body.size() - sizeof(int);
    std::memcpy(&data, m_body.data() + i, sizeof(int));
    m_body.resize(i);
    m_header.size = i;
    return *this;
}

//write uint64
Net::Message &Net::Message::operator<<(int64_t data)
{
    size_t i = m_body.size();
    m_body.resize(i + sizeof(uint64_t));
    std::memcpy(m_body.data() + i, &data, sizeof(uint64_t));
    m_header.size = i + sizeof(uint64_t);
    return *this;
}

//read uint64
Net::Message &Net::Message::operator>>(int64_t &data)
{
    size_t i = m_body.size() - sizeof(int64_t);
    std::memcpy(&data, m_body.data() + i, sizeof(int64_t));
    m_body.resize(i);
    m_header.size = i;
    return *this;
}

//write float
Net::Message &Net::Message::operator<<(float data)
{
    size_t i = m_body.size();
    m_body.resize(i + sizeof(float));
    std::memcpy(m_body.data() + i, &data, sizeof(float));
    m_header.size = i + sizeof(float);
    return *this;
}

//read float
Net::Message &Net::Message::operator>>(float &data)
{
    size_t i = m_body.size() - sizeof(float);
    std::memcpy(&data, m_body.data() + i, sizeof(float));
    m_body.resize(i);
    m_header.size = i;
    return *this;
}

//write double
Net::Message &Net::Message::operator<<(double data)
{
    size_t i = m_body.size();
    m_body.resize(i + sizeof(double));
    std::memcpy(m_body.data() + i, &data, sizeof(double));
    m_header.size = i + sizeof(double);
    return *this;
}

//read double
Net::Message &Net::Message::operator>>(double &data)
{
    size_t i = m_body.size() - sizeof(double);
    std::memcpy(&data, m_body.data() + i, sizeof(double));
    m_body.resize(i);
    m_header.size = i;
    return *this;
}

//write char
Net::Message &Net::Message::operator<<(char data)
{
    m_body.push_back(data);
    m_header.size = m_body.size();
    return *this;
}

//read char
Net::Message &Net::Message::operator>>(char &data)
{
    data = m_body.back();
    m_body.pop_back();
    m_header.size = m_body.size();
    return *this;
}

//write bool
Net::Message &Net::Message::operator<<(bool data)
{
    size_t i = m_body.size();
    m_body.resize(i + sizeof(bool));
    std::memcpy(m_body.data() + i, &data, sizeof(bool));
    m_header.size = i + sizeof(bool);
    return *this;
}

Net::Message &Net::Message::operator>>(bool &data)
{
    size_t i = m_body.size() - sizeof(data);
    std::memcpy(&data, m_body.data() + i, sizeof(data));
    m_body.resize(i);
    m_header.size = i;
    return *this;
}

//write string
Net::Message &Net::Message::operator<<(const std::string &data)
{
    size_t size = data.size();
    for (int i = 0; i < size; i++)
    {
        *this << data[i];
    }
    *this << (int)size;

    return *this;
}

//read string
Net::Message &Net::Message::operator>>(std::string &data)
{
    unsigned int size;
    *this >> (int &)size;
    size_t t = m_body.size() - size;
    data.clear();
    data.reserve(t);

    //add char individually
    for (unsigned int i = 0; i < size; i++)
        data += *(m_body.data() + t + i);
    // clear data
    for (unsigned int i = 0; i < size; i++)
        m_body.pop_back();

    m_header.size = m_body.size();
    return *this;
}

/*
old: directly memcpy in string can be dangerous

//write string
Net::Message &Net::Message::operator<<(const std::string &data)
{
    size_t size = data.size();
    size_t i = m_body.size();
    m_body.resize(i + size);
    std::memcpy(m_body.data() + i, data.data(), data.size());
    *this << (int)size;

    return *this;
}

//read string
Net::Message &Net::Message::operator>>(std::string &data)
{
    unsigned int size;
    *this >> (int&)size;
    size_t i = m_body.size() - size;
    std::memcpy(data.data(), m_body.data() + i, size);
    m_header.size = i;    

    return *this;
}
*/