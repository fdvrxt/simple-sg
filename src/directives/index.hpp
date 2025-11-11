#ifndef INDEX_HPP_
#define INDEX_HPP_

#include <filesystem>
#include <functional>

#include <nlohmann/json.hpp>
#include <inja.hpp>

#include "directive.hpp"
#include "../utils/debug.hpp"
#include "../data/config.hpp"
#include "../utils/utils.hpp"

class Index : public Directive {
public:
    Index();
    virtual void init(Config& config, const nlohmann::json directive);
    virtual void render();

    using AugmentRenderData = std::function<void(nlohmann::json&, std::size_t, std::size_t)>;

    static void render_paginated(
        Config& config,
        const inja::Template& temp,
        const nlohmann::json& base_data,
        const nlohmann::json& pages,
        int count,
        const std::filesystem::path& output_dir,
        const AugmentRenderData& augment = nullptr
    );
};

#endif
