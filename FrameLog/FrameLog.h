#pragma once

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
  #ifdef FRAMELOG_EXPORTS
  #define FRAMELOG_API __declspec(dllexport)
  #else
  #define FRAMELOG_API __declspec(dllimport)
  #endif
#else
  #define FRAMELOG_API __attribute__((visibility("default")))
#endif

// 无 Qt 依赖的轻量日志核心。
// 作为共享库提供进程级单例，供主程序与业务 DLL(含未来 App/Bin 下插件)共享同一份日志状态。
namespace vflog
{
    enum class Level : int
    {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    FRAMELOG_API const char* levelName(Level lvl);
    FRAMELOG_API Level levelFromName(const char* name);

    // 单条日志记录（缓冲与回调均持有所有权的字符串，跨 DLL 回传安全）。
    struct Record
    {
        uint64_t    timeMs{ 0 };
        Level       level{ Level::Info };
        std::string category;
        std::string message;
        std::string file;
        std::string func;
        int         line{ 0 };
        uint32_t    threadId{ 0 };
    };

    struct Config
    {
        Level       minLevel{ Level::Info };
        std::string logDir;                           // 为空则回退到 exe 目录/logs
        std::string filePrefix{ "VisionFrame" };
        int64_t     maxFileBytes{ 10 * 1024 * 1024 }; // 单文件超过此大小即滚动分片
        int         maxBackupFiles{ 10 };             // 单日滚动保留份数
        int         maxDayFiles{ 30 };                // 按天保留天数(<=0 表示不限)
        bool        enableFile{ true };               // 是否写文件 sink
        bool        enableDebugOutput{ true };        // 是否写 OutputDebugString
    };

    // UI 面板订阅回调（实现在 exe 的 Qt 层，DLL 保持无 Qt）。
    using SinkFn = std::function<void(const Record&)>;

    class FRAMELOG_API Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static Logger& instance();

        void configure(const Config& cfg);
        Config config() const;

        void  setMinLevel(Level lvl);
        Level minLevel() const;

        // 按分类单独覆盖级别（例如把 "image" 调到 Trace）；分类不存在则回退全局 minLevel。
        void setCategoryLevel(const std::string& category, Level lvl);
        void clearCategoryLevels();

        // UI 订阅与环形历史缓冲（面板晚开时可批量回填）。
        void setUiSink(SinkFn fn, size_t historyCapacity = 5000);
        void clearUiSink();
        std::vector<Record> history() const;

        // 强制刷盘（崩溃处理或退出前调用）。
        void flush();

        // 线程安全核心入口，通常由 VFL_* 宏调用。
        void log(Level lvl, const char* category, const std::string& msg,
                 const char* file, const char* func, int line);

    private:
        Logger();
        ~Logger();

        class Impl;
        Impl* m_impl;
    };

    // 流式辅助对象：在表达式末尾析构时统一提交到 Logger。
    class FRAMELOG_API LogStream
    {
    public:
        LogStream(Level lvl, const char* category, const char* file, const char* func, int line);
        ~LogStream();

        LogStream(LogStream&&) = default;
        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;
        LogStream& operator=(LogStream&&) = delete;

        template <typename T>
        LogStream& operator<<(const T& v)
        {
            m_oss << v;
            return *this;
        }

        LogStream& operator<<(const char* s)
        {
            m_oss << (s ? s : "(null)");
            return *this;
        }

        // 支持 std::endl/std::flush 等流操纵符
        LogStream& operator<<(std::ostream& (*pf)(std::ostream&))
        {
            m_oss << pf;
            return *this;
        }

    private:
        Level              m_level;
        const char*        m_category;
        const char*        m_file;
        const char*        m_func;
        int                m_line;
        std::ostringstream m_oss;
    };
} // namespace vflog

// 日志宏。用法：VFL_INFO("task") << "message " << 123;
#define VFL_LOG(level, category) \
    ::vflog::LogStream((level), (category), __FILE__, __FUNCTION__, __LINE__)

#define VFL_TRACE(category) VFL_LOG(::vflog::Level::Trace, (category))
#define VFL_DEBUG(category) VFL_LOG(::vflog::Level::Debug, (category))
#define VFL_INFO(category)  VFL_LOG(::vflog::Level::Info,  (category))
#define VFL_WARN(category)  VFL_LOG(::vflog::Level::Warn,  (category))
#define VFL_ERROR(category) VFL_LOG(::vflog::Level::Error, (category))
#define VFL_FATAL(category) VFL_LOG(::vflog::Level::Fatal, (category))