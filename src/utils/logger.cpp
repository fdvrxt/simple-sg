#include "logger.hpp"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : stopLogging_(false), logThread_(&Logger::processLogs, this) {}

Logger::~Logger() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stopLogging_ = true;
    }
    cv_.notify_one();
    logThread_.join();
}

void Logger::log(LogLevel level, const std::ostringstream& message) {
    getInstance().enqueueLog(level, message.str());
}

void Logger::enqueueLog(LogLevel level, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        logQueue_.emplace(level, message);
    }
    cv_.notify_one();
}

void Logger::processLogs() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        cv_.wait(lock, [this] { return !logQueue_.empty() || stopLogging_; });

        while (!logQueue_.empty()) {
            auto [level, message] = logQueue_.front();
            logQueue_.pop();
            lock.unlock();

            printLog(level, message);
            lock.lock();
        }

        if (stopLogging_) break;
    }
}

void Logger::printLog(LogLevel level, const std::string& message) {
    std::string color;
    std::string levelText;
    std::ostream* outputStream = &std::cout;

    switch (level) {
        case LogLevel::INFO:
            levelText = "INFO:";
            break;
        case LogLevel::WARNING:
            color = YELLOW;
            levelText = "WARNING:";
            break;
        case LogLevel::ERROR:
            color = RED;
            levelText = "ERROR:";
            outputStream = &std::cerr; // redirect to std::cerr for errors
            break;
    }

    *outputStream << color << levelText << " " << message << RESET << std::endl;
}
