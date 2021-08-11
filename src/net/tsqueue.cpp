#include "net/tsqueue.hpp"
#include "net/message.hpp"

template <typename T>
const T &Net::tsqueue<T>::front()
{
    std::scoped_lock(m_mutex);
    return m_queue.front();
}

template <typename T>
const T &Net::tsqueue<T>::back()
{
    std::scoped_lock(m_mutex);
    return m_queue.back();
}

template <typename T>
std::deque<T> &Net::tsqueue<T>::getQueue()
{
    return std::ref(m_queue);
}

template <typename T>
T Net::tsqueue<T>::pop_front()
{
    std::scoped_lock lock(m_mutex);
    auto t = std::move(m_queue.front());
    m_queue.pop_front();
    return t;
}

template <typename T>
T Net::tsqueue<T>::pop_back()
{
    std::scoped_lock lock(m_mutex);
    auto t = std::move(m_queue.back());
    m_queue.pop_back();
    return t;
}

template <typename T>
void Net::tsqueue<T>::push_back(const T &item)
{
    std::scoped_lock lock(m_mutex);
    m_queue.emplace_back(std::move(item));
}

template <typename T>
void Net::tsqueue<T>::push_front(const T &item)
{
    std::scoped_lock lock(m_mutex);
    m_queue.emplace_front(std::move(item));
}

template <typename T>
bool Net::tsqueue<T>::empty()
{
    std::scoped_lock lock(m_mutex);
    return m_queue.empty();
}

template <typename T>
size_t Net::tsqueue<T>::count()
{
    std::scoped_lock lock(m_mutex);
    return m_queue.size();
}

template <typename T>
void Net::tsqueue<T>::clear()
{
    std::scoped_lock lock(m_mutex);
    m_queue.clear();
}

template class Net::tsqueue<Net::Message>;
template class Net::tsqueue<Net::Owned_message>;