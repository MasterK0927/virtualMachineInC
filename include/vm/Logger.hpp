#pragma once

#include <string>
#include <iostream>
#include <memory>

namespace vm {

struct ILogger {
    virtual ~ILogger() = default;
    virtual void info(const std::string& msg) = 0;
    virtual void warn(const std::string& msg) = 0;
    virtual void error(const std::string& msg) = 0;
};

struct ConsoleLogger : public ILogger {
    void info(const std::string& msg) override { std::cout << "[INFO] " << msg << std::endl; }
    void warn(const std::string& msg) override { std::cout << "[WARN] " << msg << std::endl; }
    void error(const std::string& msg) override { std::cerr << "[ERROR] " << msg << std::endl; }
};

enum class LogSeverity { Error = 0, Warn = 1, Info = 2 };

// Filters messages below a minimum severity, forwarding to an underlying ILogger
class FilteredLogger : public ILogger {
public:
    FilteredLogger(ILogger& delegate, LogSeverity minSeverity)
        : m_delegate(delegate), m_min(minSeverity) {}

    void info(const std::string& msg) override {
        if (static_cast<int>(m_min) <= static_cast<int>(LogSeverity::Info)) m_delegate.info(msg);
    }
    void warn(const std::string& msg) override {
        if (static_cast<int>(m_min) <= static_cast<int>(LogSeverity::Warn)) m_delegate.warn(msg);
    }
    void error(const std::string& msg) override {
        m_delegate.error(msg);
    }

private:
    ILogger& m_delegate;
    LogSeverity m_min{LogSeverity::Info};
};

inline LogSeverity parseSeverity(const std::string& s) {
    if (s == "error") return LogSeverity::Error;
    if (s == "warn") return LogSeverity::Warn;
    return LogSeverity::Info;
}

// Buffered logger keeps last N lines for GUI consoles while optionally forwarding.
class BufferedLogger : public ILogger {
public:
    explicit BufferedLogger(std::size_t capacity = 1024, ILogger* forward = nullptr)
        : m_capacity(capacity), m_forward(forward) {}

    void setForward(ILogger* fwd) { m_forward = fwd; }
    const std::vector<std::string>& lines() const { return m_lines; }
    void clear() { m_lines.clear(); }

    void info(const std::string& msg) override { add("[INFO] " + msg); if (m_forward) m_forward->info(msg); }
    void warn(const std::string& msg) override { add("[WARN] " + msg); if (m_forward) m_forward->warn(msg); }
    void error(const std::string& msg) override { add("[ERROR] " + msg); if (m_forward) m_forward->error(msg); }

private:
    void add(const std::string& line) {
        if (m_lines.size() >= m_capacity) {
            m_lines.erase(m_lines.begin(), m_lines.begin() + (m_capacity / 4)); // drop 25%
        }
        m_lines.push_back(line);
    }
    std::size_t m_capacity;
    ILogger* m_forward{nullptr};
    std::vector<std::string> m_lines;
};

} // namespace vm
