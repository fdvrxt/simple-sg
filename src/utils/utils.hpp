#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <sstream>
#include <optional>
#include <md4c-html.h>
#include <filesystem>
#include <mutex>
#include <fstream>
#include "logger.hpp"
#include "debug.hpp"

#define LOG(level, msg) { std::ostringstream oss; oss << msg; Logger::log(level, oss); }
#define LOG_INFO(msg)  LOG(LogLevel::INFO, msg)
#define LOG_WARN(msg)  LOG(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) LOG(LogLevel::ERROR, msg)

namespace utils {
    extern std::mutex logMutex;

    enum ExtractionType {
        FRONTMATTER = 1 << 0,
        MARKDOWN = 1 << 1
    };

    std::pair<std::optional<std::string>, std::optional<std::string>> extract(const std::string& str, int extraction_type);
    std::filesystem::path   getOutputPath(
        const std::filesystem::path& content_dir,
        const std::filesystem::path& output_dir,
        const std::filesystem::path& target
    );
    std::string             getOutputUrl(
        const std::filesystem::path& content_dir,
        const std::string& url_base,
        const std::filesystem::path& target
    );
    std::streamsize         getFileLen(std::ifstream& file);
    std::string             trim(const std::string& str);
    std::string             fetch_stream();
    bool                    output_file(const std::string& str, std::filesystem::path& file_path);
    void                    handle_md(const MD_CHAR* stuff, MD_SIZE size, void* data);
    bool                    output_file(const std::string& str, std::filesystem::path& file_path);
    void                    clear_directory(const std::filesystem::path& dir);
}

#endif
