#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>
#include <string>

#include "../utils/utils.hpp"
#include "builder.hpp"

Builder::Builder(Feeder& feeder) :
    feeder(feeder) {}

Builder::~Builder() { }

void Builder::build() {
    unsigned int num_threads = std::thread::hardware_concurrency();
    LOG_INFO("Building with: " << num_threads << " threads");

    std::vector<Page> processed_pages;
    start_content_threads(num_threads, processed_pages);

    Config& config = feeder.getConfig();
    std::filesystem::path output_dir = config.getSiteDirectory() / "output";
    utils::clear_directory(output_dir);

    collect_and_validate_pages(processed_pages, config);
    sort_and_store_pages(config);
    start_render_threads(num_threads, processed_pages, config);
    copy_theme_assets(config);
}


void Builder::start_content_threads(unsigned int num_threads, std::vector<Page>& processed_pages) {
    std::vector<std::thread> threads;
    std::mutex processed_pages_mutex;

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &processed_pages, &processed_pages_mutex] {
            content_worker_thread(processed_pages, processed_pages_mutex);
        });
    }
    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
}

void Builder::content_worker_thread(std::vector<Page>& processed_pages, std::mutex& processed_pages_mutex) {
    Config& config = feeder.getConfig();
    std::optional<std::pair<std::int32_t, std::filesystem::path>> page_path_opt = feeder.getNext();

    while (page_path_opt.has_value()) {
        auto [index, page_path] = page_path_opt.value();
        LOG_INFO("Processing content (index: " << index << "): " << page_path);

        try {
            auto [markdown, frontmatter] = read_and_extract(page_path);
            Data page_data((std::string(frontmatter)));
            std::string html = generate_html(markdown);
            page_data.set<std::string>(html, "content");

            std::string output_path = utils::getOutputPath(
                config.getSiteDirectory() / "content",
                config.getSiteDirectory() / "output",
                page_path
            ).string();
            page_data.set<std::string>(output_path, "path");

            std::string output_url = utils::getOutputUrl(
                config.getSiteDirectory() / "content",
                config.getData().get<std::string>("site", "url"),
                page_path
            );
            page_data.set<std::string>(output_url, "url");

            {
                std::lock_guard<std::mutex> lock(processed_pages_mutex);
                processed_pages.push_back(Page(page_data));
            }
            LOG_INFO("Finished processing content (index: " << index << ")");
        } catch (const std::exception& e) {
            LOG_ERROR("Error processing content: "  << page_path);
            LOG_ERROR("Error message: "             << e.what());
        }
        page_path_opt = feeder.getNext();
    }
}

void Builder::start_render_threads(unsigned int num_threads, std::vector<Page>& processed_pages, Config& config) {
    std::vector<std::thread> threads;
    std::atomic<size_t> page_index{0};
    
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&processed_pages, &config, &page_index] {
            while (true) {
                size_t idx = page_index.fetch_add(1);
                if (idx >= processed_pages.size()) break;
                processed_pages[idx].render(config);
            }
        });
    }
    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
}

std::pair<std::string, std::string> Builder::read_and_extract(const std::filesystem::path& page_path) {
    std::ifstream markdown_file(page_path);
    if (!markdown_file.is_open()) {
        throw std::runtime_error("Failed to open markdown file: " + page_path.string());
    }

    std::stringstream file_content_stream;
    std::string line;
    while (std::getline(markdown_file, line)) {
        file_content_stream << line << '\n';
    }
    std::string file_content = file_content_stream.str();

    auto extracted = utils::extract(file_content, utils::MARKDOWN | utils::FRONTMATTER);
    if (!extracted.first.has_value() || !extracted.second.has_value()) {
        throw std::runtime_error("Failed to extract markdown and frontmatter from file: " + page_path.string());
    }
    return {extracted.first.value(), extracted.second.value()};
}

std::string Builder::generate_html(const std::string_view& markdown) {
    std::stringstream html_stream;
    md_html(markdown.data(), markdown.length(), utils::handle_md, &html_stream, 0, 0);
    return html_stream.str();
}

void Builder::collect_and_validate_pages(std::vector<Page>& processed_pages, Config& config) {
    for (auto& page : processed_pages) {
        page.validate(config);
        config.getData().add(page.getPageData(), "pages");
    }
}

void Builder::sort_and_store_pages(Config& config) {
    nlohmann::json pages = config.getData().getJson()["pages"];
    std::sort(
        pages.begin(), pages.end(),
        [](const nlohmann::json& a, const nlohmann::json& b) {
            return static_cast<time_t>(a["timestamp"]) > static_cast<time_t>(b["timestamp"]);
        }
    );
    config.getData().set<nlohmann::json>(pages, "site", "pages");
}

void Builder::render_pages(std::vector<Page>& processed_pages, Config& config) {
    for (auto& page : processed_pages) {
        page.render(config);
    }
}

void Builder::copy_theme_assets(Config& config) {
    std::filesystem::path theme_dir = config.getThemeDirectory();
    std::filesystem::path assets_dir = theme_dir / config.getData().get<std::string>("theme", "assets-directory");
    std::filesystem::path relative_path = std::filesystem::relative(assets_dir, theme_dir);
    std::filesystem::path build_dir = config.getSiteDirectory() / "output";
    std::filesystem::path target_assets_dir = build_dir / relative_path;

    std::filesystem::copy(
        assets_dir,
        target_assets_dir,
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
    );
}