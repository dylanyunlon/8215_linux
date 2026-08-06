#ifndef __ORDEREDLOCK_H__
#define __ORDEREDLOCK_H__
#include <condition_variable>
#include <mutex>

namespace universal_utils {
    class OrderedLock
    {
    public:
        OrderedLock() = default;
        OrderedLock(const OrderedLock &obj) = delete;
        OrderedLock(OrderedLock&&) = delete;

        virtual ~OrderedLock()
        {
            closeOrderdLock();
        }

        void lock()
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            size_t orderNumber = m_maxOrder++;
            while (orderNumber != m_curOrder) {
                m_conditionLock.wait(lck);
            }
        }

        void unlock()
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            m_curOrder++;

            m_conditionLock.notify_all();
        }
    private:
        void closeOrderdLock()
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            m_conditionLock.notify_all();
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_conditionLock;
        size_t m_curOrder = 0;
        size_t  m_maxOrder = 0;
    };
}
#endif
