#include "FrameDataQueueRegistry.h"

FrameDataQueueRegistry& FrameDataQueueRegistry::instance()
{
    static FrameDataQueueRegistry s_instance;
    return s_instance;
}

std::shared_ptr<FrameDataQueue> FrameDataQueueRegistry::getOrCreate(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(name);
    if (it != m_map.end())
        return it->second;
    // 不存在则新建并登记
    auto queue = std::make_shared<FrameDataQueue>();
    m_map.emplace(name, queue);
    return queue;
}

std::shared_ptr<FrameDataQueue> FrameDataQueueRegistry::get(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(name);
    if (it != m_map.end())
        return it->second;
    return nullptr;
}

void FrameDataQueueRegistry::registerQueue(const std::string& name, std::shared_ptr<FrameDataQueue> queue)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_map[name] = std::move(queue);
}

void FrameDataQueueRegistry::remove(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_map.erase(name);
}

std::vector<std::string> FrameDataQueueRegistry::names() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> result;
    result.reserve(m_map.size());
    for (const auto& kv : m_map)
        result.push_back(kv.first);
    return result;
}
