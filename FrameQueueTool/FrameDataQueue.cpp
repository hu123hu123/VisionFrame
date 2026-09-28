#include "FrameDataQueue.h"

FrameDataQueue::~FrameDataQueue()
{
    stop();
}

void FrameDataQueue::enqueue(std::any data)
{
    QMutexLocker lock(&m_mutex);
    if (m_stop.load(std::memory_order_relaxed))
        return; // 已停止，丢弃入队数据
    m_queue.enqueue(std::move(data));
    m_cond.wakeOne();
}

bool FrameDataQueue::dequeue(std::any& out, int timeoutMs)
{
    QMutexLocker lock(&m_mutex);
    if (timeoutMs < 0)
    {
        // 无限等待，直到有数据或被 stop 唤醒
        while (m_queue.isEmpty() && !m_stop.load(std::memory_order_relaxed))
            m_cond.wait(&m_mutex);
    }
    else
    {
        // 带超时等待
        unsigned long remaining = static_cast<unsigned long>(timeoutMs);
        while (m_queue.isEmpty() && !m_stop.load(std::memory_order_relaxed) && remaining > 0)
            remaining = m_cond.wait(&m_mutex, remaining);
    }

    if (m_queue.isEmpty())
        return false; // 超时或被 stop 唤醒

    out = std::move(m_queue.dequeue());
    return true;
}

bool FrameDataQueue::tryDequeue(std::any& out)
{
    QMutexLocker lock(&m_mutex);
    if (m_queue.isEmpty())
        return false;
    out = std::move(m_queue.dequeue());
    return true;
}

void FrameDataQueue::clear()
{
    QMutexLocker lock(&m_mutex);
    m_queue.clear();
}

int FrameDataQueue::size() const
{
    QMutexLocker lock(&m_mutex);
    return m_queue.size();
}

bool FrameDataQueue::isEmpty() const
{
    QMutexLocker lock(&m_mutex);
    return m_queue.isEmpty();
}

void FrameDataQueue::stop()
{
    QMutexLocker lock(&m_mutex);
    m_stop.store(true, std::memory_order_relaxed);
    m_cond.wakeAll();
}

void FrameDataQueue::reset()
{
    QMutexLocker lock(&m_mutex);
    m_stop.store(false, std::memory_order_relaxed);
}
