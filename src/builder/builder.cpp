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

void Builder::worker_thread(std::vector<Page>& processed_pages) {
    Config& config = feeder.getConfig();

    std::stringstream thread_html_stream;
    std::optional<std::pair<std::int32_t, std::filesystem::path>> page_path_opt = feeder.getNext();

    while (page_path_opt.has_value()) {
        std::pair<std::int32_t, std::filesystem::path> page_entry = page_path_opt.value();
        std::int32_t index = page_entry.first;
        std::filesystem::path page_path = page_entry.second;

        LOG_INFO("Processing content (index: " << index << "): " << page_path);

        try {
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

            // extract markdown and frontmatter
            auto extracted = utils::extract(file_content, utils::MARKDOWN | utils::FRONTMATTER);

            if (!extracted.first.has_value() || !extracted.second.has_value())
            {
                throw std::runtime_error("Failed to extract markdown and frontmatter from file: " + page_path.string());
            }

            std::string_view markdown    = extracted.first.value();
            std::string_view frontmatter = extracted.second.value();

            Data page_data((std::string(frontmatter)));

            // convert markdown to html
            md_html(markdown.data(), markdown.length(), utils::handle_md, &thread_html_stream, 0, 0);

            // fetch html from stream and save the output
            std::string html = thread_html_stream.str();
            page_data.set<std::string>(html, "content");

            // compute and save the path
            std::string output_path = utils::getOutputPath(
                config.getSiteDirectory() / "content",
                config.getSiteDirectory() / "output",
                page_path
            ).string();
            page_data.set<std::string>(output_path, "path");

            // compute and save the url
            std::string output_url = utils::getOutputUrl(
                config.getSiteDirectory() / "content",
                config.getData().get<std::string>("site", "url"),
                page_path
            );
            page_data.set<std::string>(output_url, "url");
            
            // save page
            Page page(page_data);
            processed_pages.push_back(page);

            LOG_INFO("Finished processing content (index: " << index << ")");

        } catch (const std::exception& e) {
            LOG_ERROR("Error processing content: "  << page_path);
            LOG_ERROR("Error message: "             << e.what());
        }

        thread_html_stream.str("");
        thread_html_stream.clear(); 
        page_path_opt = feeder.getNext();
    }
}

void Builder::build() {
    unsigned int num_threads = std::thread::hardware_concurrency();
    LOG_INFO("Building with: " << num_threads << " threads");

    std::vector<std::thread> threads;
    std::vector<Page> processed_pages;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &processed_pages] { worker_thread(processed_pages); });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    Config& config = feeder.getConfig();

    std::filesystem::path output_dir = config.getSiteDirectory() / "output";
    utils::clear_directory(output_dir);

    for (auto& page : processed_pages) {
        page.validate(config);
        config.getData().add(page.getPageData(), "pages");
    }

    nlohmann::json pages = config.getData().getJson()["pages"];

    // at this point it's guaranteed that each page has a timestamp
    std::sort(
        pages.begin(),
        pages.end(),
        [] (const nlohmann::json& a, const nlohmann::json& b) {
            time_t ts_a = a["timestamp"];
            time_t ts_b = b["timestamp"];

            return ts_a > ts_b;
        }
    );

    config.getData().set<nlohmann::json>(pages, "site", "pages");

    for (auto& page : processed_pages) {
        page.render(config);
    }
    
    // TODO move to config.cpp
    std::filesystem::path theme_dir         = config.getThemeDirectory();
    std::filesystem::path assets_dir        = theme_dir / config.getData().get<std::string>("theme", "assets-directory");
    std::filesystem::path relative_path     = std::filesystem::relative(assets_dir, theme_dir);
    std::filesystem::path build_dir         = config.getSiteDirectory() / "output";
    std::filesystem::path target_assets_dir = build_dir / relative_path;

    std::filesystem::copy(
        assets_dir,
        target_assets_dir,
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing
    );
}
