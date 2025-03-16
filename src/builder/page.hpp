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
    void render(Config& config);

    const Data& getPageData() const { return page_data; }
    bool operator<(const Page& other) const;
};

#endif
