#include "pch.h"
#include "FrameLog.h"
#include "OsCompat.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>

namespace vflog
{
    const char* levelName(Level lvl)
    {
        switch (lvl)
        {
        case Level::Trace: return "TRACE";
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO";
        case Level::Warn:  return "WARN";
        case Level::Error: return "ERROR";
        case Level::Fatal: return "FATAL";
        }
        return "INFO";
    }

    Level levelFromName(const char* name)
    {
        if (!name)
            return Level::Info;
        for (int i = (int)Level::Trace; i <= (int)Level::Fatal; ++i)
        {
            if (os::strcasecmpSafe(levelName((Level)i), name) == 0)
                return (Level)i;
        }
        return Level::Info;
    }

    namespace
    {
        using Clock = std::chrono::system_clock;

        uint64_t nowMs()
        {
            return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now().time_since_epoch()).count();
        }

        struct TmParts
        {
            int year, mon, day, hour, min, sec, ms;
        };

        TmParts splitTime(uint64_t ms)
        {
            std::time_t t = (std::time_t)(ms / 1000);
            std::tm tmb{};
            os::localtimeSafe(t, &tmb);
            TmParts p{};
            p.year = tmb.tm_year + 1900;
            p.mon = tmb.tm_mon + 1;
            p.day = tmb.tm_mday;
            p.hour = tmb.tm_hour;
            p.min = tmb.tm_min;
            p.sec = tmb.tm_sec;
            p.ms = (int)(ms % 1000);
            return p;
        }

        std::string formatTime(uint64_t ms)
        {
            TmParts p = splitTime(ms);
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                     p.year, p.mon, p.day, p.hour, p.min, p.sec, p.ms);
            return buf;
        }

        std::string dateTag(uint64_t ms)
        {
            TmParts p = splitTime(ms);
            char buf[16];
            snprintf(buf, sizeof(buf), "%04d%02d%02d", p.year, p.mon, p.day);
            return buf;
        }

        // 序列号归档名：xxx.log -> xxx_<n>.log
        std::string indexedPath(const std::string& path, int n)
        {
            std::string::size_type dot = path.rfind('.');
            const std::string stem = (dot == std::string::npos) ? path : path.substr(0, dot);
            const std::string suffix = (dot == std::string::npos) ? std::string() : path.substr(dot);
            return stem + "_" + std::to_string(n) + suffix;
        }

        // 默认为 exe 同级的 logs 目录（路径分隔符统一 '/'，Windows 同样接受）
        std::string defaultLogDir()
        {
            return os::exeDirUtf8() + "/logs";
        }
    } // namespace

    class Logger::Impl
    {
    public:
        Config cfg;
        std::map<std::string, Level> categoryLevels;
        std::deque<Record> history;
        size_t historyCapacity{ 5000 };
        SinkFn uiSink;

        mutable std::mutex mutex;

        std::ofstream file;
        std::string    filePath;
        std::string    fileDate;
        uint64_t       fileBytes{ 0 };
        bool           logDirReady{ false };

        void configureLocked(const Config& c)
        {
            const bool dirChanged = (cfg.logDir != c.logDir) || (cfg.filePrefix != c.filePrefix);
            cfg = c;
            categoryLevels.clear();
            if (dirChanged)
            {
                if (file.is_open())
                {
                    file.flush();
                    file.close();
                }
                filePath.clear();
                fileDate.clear();
                fileBytes = 0;
                logDirReady = false;
            }
        }

        bool levelEnabled(Level lvl, const char* category) const
        {
            Level min = cfg.minLevel;
            if (category && *category)
            {
                auto it = categoryLevels.find(category);
                if (it != categoryLevels.end())
                    min = it->second;
            }
            return (int)lvl >= (int)min;
        }

        void ensureLogDir()
        {
            if (cfg.logDir.empty() && !logDirReady)
            {
                cfg.logDir = defaultLogDir();
            }
            if (logDirReady || cfg.logDir.empty())
                return;
            std::error_code ec;
            std::filesystem::create_directories(cfg.logDir, ec);
            logDirReady = true;
            cleanupOldFiles();
        }

        void ensureOpenFile(const Record& r)
        {
            if (!cfg.enableFile)
                return;
            ensureLogDir();
            if (cfg.logDir.empty())
                return;

            const std::string date = dateTag(r.timeMs);
            const std::string path = cfg.logDir + "\\" + cfg.filePrefix + "_" + date + ".log";

            if (file.is_open() && path == filePath && fileBytes < (uint64_t)cfg.maxFileBytes)
                return;

            if (file.is_open())
            {
                file.flush();
                file.close();
            }

            std::error_code ec;
            uintmax_t existing = 0;
            if (std::filesystem::exists(path, ec))
                existing = std::filesystem::file_size(path, ec);

            if (existing >= (uint64_t)cfg.maxFileBytes)
            {
                rollFile(path);
                existing = 0;
            }

            file.open(path, std::ios::out | std::ios::app);
            if (file.is_open())
            {
                filePath = path;
                fileDate = date;
                fileBytes = existing;
            }
        }

        void rollFile(const std::string& path)
        {
            if (cfg.maxBackupFiles <= 0)
            {
                std::error_code ec;
                std::filesystem::remove(path, ec);
                return;
            }

            std::error_code ec;
            // 淘汰最旧归档
            std::filesystem::remove(indexedPath(path, cfg.maxBackupFiles), ec);
            // 后续归档序号依次 +1（从大到小，避免覆盖）
            for (int n = cfg.maxBackupFiles - 1; n >= 1; --n)
            {
                std::string src = indexedPath(path, n);
                if (std::filesystem::exists(src, ec))
                    std::filesystem::rename(src, indexedPath(path, n + 1), ec);
            }
            std::filesystem::rename(path, indexedPath(path, 1), ec);
        }

        void cleanupOldFiles()
        {
            if (cfg.maxDayFiles <= 0 || cfg.logDir.empty())
                return;
            std::error_code ec;
            auto cutoff = std::filesystem::file_time_type::clock::now()
                - std::chrono::hours(24 * cfg.maxDayFiles);
            for (std::filesystem::directory_iterator it(cfg.logDir, ec), end; !ec && it != end; it.increment(ec))
            {
                if (ec)
                    break;
                const std::string name = it->path().filename().string();
                if (name.rfind(cfg.filePrefix + "_", 0) != 0)
                    continue;
                if (name.size() < 4 || name.substr(name.size() - 4) != ".log")
                    continue;
                if (it->last_write_time(ec) < cutoff)
                    std::filesystem::remove(it->path(), ec);
            }
        }

        void writeFile(const Record& r)
        {
            if (!cfg.enableFile)
                return;
            ensureOpenFile(r);
            if (!file.is_open())
                return;

            std::string line;
            line.reserve(160 + r.message.size());
            line += formatTime(r.timeMs);
            line += " [";
            line += levelName(r.level);
            line += "] [";
            line += r.category;
            line += "] [";
            line += std::to_string(r.threadId);
            line += "] ";
            line += r.message;
            if (!r.file.empty())
            {
                line += "  -- ";
                line += r.file;
                if (r.line > 0)
                {
                    line += ":";
                    line += std::to_string(r.line);
                }
            }
            line += "\n";

            file.write(line.data(), (std::streamsize)line.size());
            fileBytes += (uint64_t)line.size();
            if (r.level >= Level::Info)
                file.flush();
        }

        void writeDebug(const Record& r)
        {
            if (!cfg.enableDebugOutput)
                return;
            std::string line;
            line.reserve(80 + r.message.size());
            line += "[VisionFrame] ";
            line += formatTime(r.timeMs);
            line += " [";
            line += levelName(r.level);
            line += "] [";
            line += r.category;
            line += "] ";
            line += r.message;
            line += "\n";
            OutputDebugStringA(line.c_str());
        }
    };

    Logger::Logger() : m_impl(new Impl()) {}
    Logger::~Logger()
    {
        if (m_impl)
        {
            if (m_impl->file.is_open())
                m_impl->file.flush();
            delete m_impl;
            m_impl = nullptr;
        }
    }

    Logger& Logger::instance()
    {
        static Logger s;
        return s;
    }

    void Logger::configure(const Config& cfg)
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->configureLocked(cfg);
    }

    Config Logger::config() const
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        return m_impl->cfg;
    }

    void Logger::setMinLevel(Level lvl)
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->cfg.minLevel = lvl;
    }

    Level Logger::minLevel() const
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        return m_impl->cfg.minLevel;
    }

    void Logger::setCategoryLevel(const std::string& category, Level lvl)
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->categoryLevels[category] = lvl;
    }

    void Logger::clearCategoryLevels()
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->categoryLevels.clear();
    }

    void Logger::setUiSink(SinkFn fn, size_t historyCapacity)
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->uiSink = std::move(fn);
        m_impl->historyCapacity = historyCapacity;
        while (m_impl->history.size() > m_impl->historyCapacity)
            m_impl->history.pop_front();
    }

    void Logger::clearUiSink()
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        m_impl->uiSink = nullptr;
    }

    std::vector<Record> Logger::history() const
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        return std::vector<Record>(m_impl->history.begin(), m_impl->history.end());
    }

    void Logger::flush()
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (m_impl->file.is_open())
            m_impl->file.flush();
    }

    void Logger::log(Level lvl, const char* category, const std::string& msg,
                     const char* file, const char* func, int line)
    {
        Record rec;
        SinkFn sink;
        {
            std::lock_guard<std::mutex> lock(m_impl->mutex);
            if (!m_impl->levelEnabled(lvl, category))
                return;

            rec.timeMs = nowMs();
            rec.level = lvl;
            rec.category = (category && *category) ? category : "app";
            rec.message = msg;
            rec.file = file ? file : "";
            rec.func = func ? func : "";
            rec.line = line;
            rec.threadId = (uint32_t)GetCurrentThreadId();

            m_impl->history.push_back(rec);
            while (m_impl->history.size() > m_impl->historyCapacity)
                m_impl->history.pop_front();

            m_impl->writeFile(rec);
            m_impl->writeDebug(rec);

            sink = m_impl->uiSink;
        }
        if (sink)
            sink(rec);
    }

    LogStream::LogStream(Level lvl, const char* category, const char* file, const char* func, int line)
        : m_level(lvl), m_category(category), m_file(file), m_func(func), m_line(line)
    {
    }

    LogStream::~LogStream()
    {
        Logger::instance().log(m_level, m_category, m_oss.str(), m_file, m_func, m_line);
    }
} // namespace vflog