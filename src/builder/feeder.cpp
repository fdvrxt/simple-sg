#include "feeder.hpp"
#include <iostream>

Feeder::Feeder(Config& config) :
    config(config), counter(0)
{
    fetchPosts();
}

void Feeder::fetchPosts() {
    std::filesystem::path content_dir = config.getSiteDirectory() / "content";

    if (!std::filesystem::exists(content_dir) || !std::filesystem::is_directory(content_dir)) {
        std::stringstream ss;
        ss << "Content directory not found or inaccessible: " << content_dir;
        throw std::runtime_error(ss.str());
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(content_dir)) {
        if (std::filesystem::is_regular_file(entry.status())) {
            LOG_INFO("Queueing content: " << entry.path());
            page_queue.push(entry.path());
        }
    }
    if (std::filesystem::is_empty(content_dir)) { } // TODO
}

bool Feeder::isQueueEmpty() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return page_queue.empty();
}

std::optional<std::pair<std::int32_t, std::filesystem::path>> Feeder::getNext() {
    std::unique_lock<std::mutex> lock(queue_mutex);

    if (page_queue.empty()) {
        return std::nullopt;
    }
    
    std::filesystem::path next_path = page_queue.front();
    page_queue.pop();
    ++counter;
    return std::pair<std::int32_t, std::filesystem::path>(counter, next_path);
}
