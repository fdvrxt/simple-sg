#ifndef PAGE_HPP_
#define PAGE_HPP_

#include "../data/data.hpp"
#include "../data/config.hpp"
#include <string>
#include <filesystem>

class Page {
private:
    Data page_data;

public:
    Page(const Data& data);
    void validate(Config& config);
    void render(Config& config, const std::string& live_reload_snippet = "");

    const Data& getPageData() const { return page_data; }
    bool operator<(const Page& other) const;

    static constexpr const char* DEFAULT_PAGE_TITLE = "Untitled Page";
    static constexpr const char* DEFAULT_PAGE_DESCRIPTION = "Content on this page is not yet described.";
};

#endif
