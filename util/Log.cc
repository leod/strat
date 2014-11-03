#include "util/Log.hh"

#include <iostream>
#include <ctime>

#ifdef WIN32
#include <windows.h> // Colored console output

#define WIN32_COLORED
#endif

using std::cout;
using std::endl;

char const* severityToString(LogSeverity severity) {
    switch (severity) {
    case SEVERITY_TRACE:
        return "trace";
    case SEVERITY_INFO:
        return "info";
    case SEVERITY_WARN:
        return "warn";
    default:
        assert(false);
    }
}

LogMessageBuilder::LogMessageBuilder(LogSeverity severity,
                                     char const* logger,
                                     char const* filename, size_t line)
    : severity(severity),
      logger(logger),
      filename(filename),
      line(line) {
}

LogMessageBuilder::~LogMessageBuilder() {
    message = stream.str();
    Log::write(*this);
}

LogSeverity LogMessageBuilder::getSeverity() const {
    return severity;
}

char const* LogMessageBuilder::getLogger() const {
    return logger;
}

char const* LogMessageBuilder::getFilename() const {
    return filename;
}

size_t LogMessageBuilder::getLine() const {
    return line;
}

char const* LogMessageBuilder::getMessage() const {
    return message.c_str();
}

void ConsoleLogSink::write(LogMessage const& message) {
#ifdef WIN32_COLORED
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO oldScreenBufferInfo;

    GetConsoleScreenBufferInfo(console, &oldScreenBufferInfo);

#   define SET_COLOR(color) SetConsoleTextAttribute(console, color);
#   define RESET_COLOR SetConsoleTextAttribute(console, \
        oldScreenBufferInfo.wAttributes)
#else 
#   define SET_COLOR(color)
#   define RESET_COLOR
#endif

    auto severityColor = [&] () {
        switch (message.getSeverity()) {
        case SEVERITY_TRACE:
            SET_COLOR(FOREGROUND_BLUE);
            break;
        case SEVERITY_INFO:
            SET_COLOR(FOREGROUND_GREEN);
            break;
        case SEVERITY_WARN:
            SET_COLOR(FOREGROUND_RED);
            break;
        default:
            assert(false);
        }
    };

    severityColor();
    cout << "[";
    RESET_COLOR;

    cout << message.getLogger();

    severityColor();
    cout << "] ";
    RESET_COLOR;

    cout << message.getMessage();
    cout << endl;

    RESET_COLOR;
}

FileLogSink::FileLogSink(std::string const& filename)
    : fstream(filename) { 
    if (!fstream.good())
        std::cerr << "Failed to open log file " << filename << " for writing."
                  << " Ignoring error." << std::endl;
}

FileLogSink::~FileLogSink() {
    fstream.close();
}

void FileLogSink::write(LogMessage const& message) {
    if (!fstream.good())
        return;

    // Write time
    time_t timeRaw;
    struct tm *timePtr;
    char timeStr[1024];

    time(&timeRaw);
    timePtr = localtime(&timeRaw);
    if (strftime(timeStr, 1024, "%Y-%m-%d %H:%M:%S", timePtr) != 0)
        fstream << timeStr;

    // Write message
    fstream << ": " << severityToString(message.getSeverity())
            << " (" << message.getLogger() << "): "
            << message.getMessage()
            << "\n";
    fstream.flush();
}

void Log::addSink(LogSink* sink) {
    sinks.push_back(std::unique_ptr<LogSink>(sink));
}

void Log::write(LogMessage const& message) {
    if (severityFilters.find(message.getLogger()) != severityFilters.end() &&
        severityFilters[message.getLogger()] > message.getSeverity())
        return;

    for (auto& sink : sinks)
        sink->write(message);
}

void Log::setSeverityFilter(std::string const& logger, LogSeverity severity) {
    severityFilters[logger] = severity; 
}

std::vector<std::unique_ptr<LogSink>> Log::sinks;
std::map<std::string, LogSeverity> Log::severityFilters;
