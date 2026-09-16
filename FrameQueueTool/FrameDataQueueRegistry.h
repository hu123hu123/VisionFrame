#pragma once

#include "FrameDataQueue.h"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>

// 进程级单例注册表，按名称管理 shared_ptr<FrameDataQueue>。
// 生产者(FrameEnQueueTool)与消费者(FrameDeQueueTool)用相同队列名拿到同一实例，
// 从而实现跨 TaskItem 线程的数据传递。
// TaskItem 把自身队列以"任务名"为键注册到此处，EnQueueTool 即可
// "选择入队到哪个任务"——目标任务名 = 目标队列名。
class FrameDataQueueRegistry
{
public:
    static FrameDataQueueRegistry& instance();

    // 取或创建命名队列。线程安全。EnQueue/DeQueue 都用此接口懒解析。
    std::shared_ptr<FrameDataQueue> getOrCreate(const std::string& name);

    // 仅查询，不创建。不存在返回 nullptr。
    std::shared_ptr<FrameDataQueue> get(const std::string& name) const;

    // 注册一个已有队列到指定名称（覆盖同名旧引用）。
    // TaskItem 用此接口把自身队列按任务名暴露给其它任务的工具。
    void registerQueue(const std::string& name, std::shared_ptr<FrameDataQueue> queue);

    void remove(const std::string& name);

    std::vector<std::string> names() const;

private:
    FrameDataQueueRegistry() = default;
    FrameDataQueueRegistry(const FrameDataQueueRegistry&) = delete;
    FrameDataQueueRegistry& operator=(const FrameDataQueueRegistry&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<FrameDataQueue>> m_map;
};
