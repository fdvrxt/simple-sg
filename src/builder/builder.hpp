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
    void worker_thread(std::vector<Page>& processed_pages);
    void render();

public:
    void build();

    Builder(Feeder& feeder);
    ~Builder();
};

#endif
