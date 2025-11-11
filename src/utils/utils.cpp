#include "utils.hpp"

namespace utils {
    std::mutex logMutex;
}

std::pair<std::optional<std::string>, std::optional<std::string>> utils::extract(const std::string& str, int extraction_type) {
    constexpr std::string_view delimiter = "---";

    size_t start = str.find(delimiter);
    std::optional<std::string> frontmatter;
    std::optional<std::string> markdown;

    if (start != std::string::npos) {
        start += delimiter.length();
        size_t end = str.find(delimiter, start);
        if (end != std::string::npos) {
            if (extraction_type & FRONTMATTER) {
                frontmatter = trim(str.substr(start, end - start));
            }

            if (extraction_type & MARKDOWN) {
                markdown = trim(str.substr(end + delimiter.length()));
            }
        } else {
            throw std::runtime_error("Missing closing delimiter in frontmatter");
        }
    }
    else // no frontmatter
    {
        if (extraction_type & MARKDOWN) {
            markdown = trim(str);
        }
    }

    return std::make_pair(markdown, frontmatter);
}


std::string utils::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void utils::handle_md(const MD_CHAR* md, MD_SIZE size, void* data) {
    std::stringstream* html_stream = static_cast<std::stringstream*>(data);
    *html_stream << std::string(static_cast<const char*>(md), size);
}

bool utils::output_file(const std::string& str, std::filesystem::path& file_path) {
    std::filesystem::path directory = file_path.parent_path();

    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec && !std::filesystem::exists(directory)) {
            return false;
        }
    }
    
    std::ofstream output_file_stream;
    output_file_stream.open(file_path, std::ios::out);
    if (!output_file_stream.is_open()) {
        return false;
    }
    output_file_stream << str;
    if (!output_file_stream.good()) {
        output_file_stream.close();
        return false;
    }
    output_file_stream.close();
    
    return true;
}

std::filesystem::path utils::getOutputPath(
    const std::filesystem::path& content_dir,
    const std::filesystem::path& output_dir,
    const std::filesystem::path& target
) 
{
    std::filesystem::path relative_path = std::filesystem::relative(target, content_dir);
    relative_path.replace_extension(".html");

    return output_dir / relative_path;
}

std::string utils::getOutputUrl(
    const std::filesystem::path& content_dir,
    const std::string& url_base,
    const std::filesystem::path& target
) 
{
    std::filesystem::path relative_path = std::filesystem::relative(target, content_dir);
    relative_path.replace_extension(".html");

    return url_base + "/" + relative_path.string();
}

void utils::clear_directory(const std::filesystem::path& dir) {
    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            std::filesystem::remove_all(entry.path());
        }
    }
}
