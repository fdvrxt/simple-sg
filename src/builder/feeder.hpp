#ifndef FEEDER_HPP_
#define FEEDER_HPP_

#include "../data/config.hpp"
#include <queue>
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <optional>

class Feeder {
private:
    Config& config;
    std::mutex queue_mutex;
    std::queue<std::filesystem::path> page_queue;

public:
    int counter;
    Feeder(Config& config);
    void fetchPosts();

    Config& getConfig() { return config; }
    std::mutex& getQueueMutex() { return queue_mutex; }

    std::optional<std::pair<std::int32_t, std::filesystem::path>> getNext();
    bool isQueueEmpty();
};

#endif
