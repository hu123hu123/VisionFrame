#pragma once

#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <any>
#include <atomic>
#include <memory>

// 线程安全的数据队列，替代裸 QQueue<std::any>*。
// 生产者(FrameEnQueueTool)调用 enqueue()，消费者(FrameDeQueueTool)调用 dequeue() 阻塞等待。
// 跨 TaskItem 线程传递数据时，生产者与消费者通过 shared_ptr 共享同一实例。
class FrameDataQueue
{
public:
    FrameDataQueue() = default;
    ~FrameDataQueue();

    // 生产者入队。唤醒一个等待中的消费者。
    void enqueue(std::any data);

    // 消费者阻塞出队。
    // timeoutMs = -1 表示无限等待；>0 表示超时返回 false。
    // 队列被 stop() 唤醒后立即返回 false（消费者据此退出线程）。
    bool dequeue(std::any& out, int timeoutMs = -1);

    // 非阻塞尝试出队，成功返回 true。
    bool tryDequeue(std::any& out);

    void clear();
    int  size() const;
    bool isEmpty() const;

    // 唤醒所有等待者并标记停止；后续 dequeue() 立即返回 false。
    void stop();
    // 清除停止标志，让队列可复用。
    void reset();

private:
    QQueue<std::any>      m_queue;
    mutable QMutex        m_mutex;
    QWaitCondition        m_cond;
    std::atomic<bool>     m_stop{ false };
};
