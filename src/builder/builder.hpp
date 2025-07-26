#ifndef BUILDER_HPP_
#define BUILDER_HPP_

#include "../data/config.hpp"
#include "page.hpp"
#include "feeder.hpp"
#include <inja.hpp>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

class Builder {
private:
    Feeder& feeder;

    std::pair<std::string, std::string> read_and_extract(const std::filesystem::path& page_path);
    std::string generate_html(const std::string_view& markdown);
    void render();

    void worker_thread(std::vector<Page>& processed_pages);
    //void content_worker_thread(std::vector<Page>& processed_pages);
    //void render_worker_thread(std::vector<Page>& processed_pages, Config& config, std::atomic<size_t>& next_idx);

    void start_worker_threads(unsigned int num_threads, std::vector<Page>& processed_pages);
    //void start_content_worker_threads(unsigned int num_threads, std::vector<Page>& processed_pages);
    //void start_render_worker_threads(unsigned int num_threads, std::vector<Page>& processed_pages, Config& config);

    void collect_and_validate_pages(std::vector<Page>& processed_pages, Config& config);
    void sort_and_store_pages(Config& config);
    void render_pages(std::vector<Page>& processed_pages, Config& config);
    void copy_theme_assets(Config& config);
public:
    void build();

    Builder(Feeder& feeder);
    ~Builder();
};

#endif
