#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <fstream>

#include "util/Print.hh"

#define STRINGIFY(a) #a

#define LOG(sev, logger) ::LogMessageBuilder(sev, STRINGIFY(logger), \
                                                   __FILE__, __LINE__)
#define TRACE(logger) LOG(::SEVERITY_TRACE, logger)
#define INFO(logger) LOG(::SEVERITY_INFO, logger)
#define WARN(logger) LOG(::SEVERITY_WARN, logger)

enum LogSeverity {
    SEVERITY_TRACE,
    SEVERITY_INFO,
    SEVERITY_WARN
};

char const* severityToString(LogSeverity);

struct LogMessageBuilder {
    LogMessageBuilder(LogSeverity, char const* logger,
                      char const* filename, size_t line);

    // Forwards the completed message to Log
    ~LogMessageBuilder();

    template<typename T>
    LogMessageBuilder& operator<<(T const& t) {
        stream << t;
        return *this;
    }

    LogSeverity getSeverity() const;
    char const* getLogger() const;
    char const* getFilename() const;
    size_t getLine() const;

    char const* getMessage() const;

private:
    LogSeverity const severity;
    char const* logger;
    char const* filename;
    size_t const line;

    std::stringstream stream;

    std::string message;
};

typedef LogMessageBuilder LogMessage;

struct LogSink {
    virtual void write(LogMessage const&) = 0;
};

struct ConsoleLogSink : public LogSink {
    virtual void write(LogMessage const&);
};

struct FileLogSink : public LogSink {
    FileLogSink(std::string const& filename);
    ~FileLogSink();

    virtual void write(LogMessage const&);

private:
    std::ofstream fstream;
};

struct Log {
    // Gives ownership of `sink' to Log
    static void addSink(LogSink* sink);

    static void write(LogMessage const&);

    static void setSeverityFilter(std::string const& logger, LogSeverity);

private:
    static std::vector<std::unique_ptr<LogSink>> sinks;
    static std::map<std::string, LogSeverity> severityFilters;
};
