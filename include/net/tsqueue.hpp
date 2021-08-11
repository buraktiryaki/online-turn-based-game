#pragma once

#include <deque>
#include <mutex>

namespace Net
{

    template <typename T>
    class tsqueue
    {
    public:
        tsqueue() = default;
        tsqueue(const tsqueue<T> &) = delete;
        ~tsqueue() { clear(); }

        const T &front();
        const T &back();
        std::deque<T> &getQueue();

        T pop_front();
        T pop_back();

        void push_back(const T &item);
        void push_front(const T &item);

        bool empty();
        size_t count();
        void clear();

    private:
        std::deque<T> m_queue;
        std::mutex m_mutex;
    };

} // namespace Net
