#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <iostream>
#include <sstream>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    static void log(LogLevel level, const std::ostringstream& message);

private:
    std::queue<std::pair<LogLevel, std::string>> logQueue_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    bool stopLogging_;
    std::thread logThread_;

    Logger();
    ~Logger();
    
    void enqueueLog(LogLevel level, const std::string& message);
    void processLogs();
    void printLog(LogLevel level, const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif
