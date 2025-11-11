#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "index.hpp"

Index::Index() { }

void Index::init(Config& config, const nlohmann::json directive)
{
    nlohmann::json data = config.getData().getJson();

    if (!directive.contains("count")) {
        throw std::runtime_error("Index directive missing required 'count' value");
    }

    int count = directive["count"].get<int>();

    if (count <= 0) {
        throw std::runtime_error("Index directive requires a positive 'count' value");
    }

    if (!data.contains("site") || !data["site"].contains("pages")) {
        LOG_WARN("No pages available for index directive");
        return;
    }

    const nlohmann::json& pages = data["site"]["pages"];
    if (!pages.is_array() || pages.empty()) {
        LOG_WARN("Index directive found no pages to render");
        return;
    }

    // make a filtered copy
    nlohmann::json pages_filtered = nlohmann::json::array();
    for (const auto& page : pages) {
        // treat missing/non-bool as false
        if (page.is_object() && page.value("indexable", false)) {
            pages_filtered.push_back(page);
        }
    }

    const inja::Template& temp = config.getTemplate(directive["name"]);

    std::filesystem::path output_dir = config.getSiteDirectory() / "output";

    render_paginated(config, temp, data, pages_filtered, count, output_dir);
}

void Index::render_paginated(
    Config& config,
    const inja::Template& temp,
    const nlohmann::json& base_data,
    const nlohmann::json& pages,
    int count,
    const std::filesystem::path& output_dir,
    const AugmentRenderData& augment
)
{
    if (!pages.is_array() || pages.empty()) {
        LOG_WARN("Index pagination received no pages to render");
        return;
    }

    if (count <= 0) {
        throw std::runtime_error("Index pagination requires a positive 'count' value");
    }

    std::size_t total_pages = (pages.size() + static_cast<std::size_t>(count) - 1) / static_cast<std::size_t>(count);

    inja::Environment& env = config.getEnvironment();

    for (std::size_t idx = 0; idx < total_pages; ++idx) {
        std::size_t start = idx * static_cast<std::size_t>(count);
        std::size_t end = std::min(start + static_cast<std::size_t>(count), pages.size());

        nlohmann::json subset = nlohmann::json::array();
        for (std::size_t i = start; i < end; ++i) {
            subset.push_back(pages[i]);
        }

        nlohmann::json render_data = base_data;
        render_data["pages"] = subset;
        render_data["site"]["pages"] = subset;

        nlohmann::json index_info;
        render_data["pages"] = subset;
        index_info["page_number"] = idx + 1;
        index_info["total_pages"] = total_pages;
        index_info["has_previous"] = idx > 0;
        index_info["count"] = pages.size();
        index_info["has_next"] = idx + 1 < total_pages;
        if (idx > 0) {
            index_info["previous_page"] = idx;
        }
        if (idx + 1 < total_pages) {
            index_info["next_page"] = idx + 2;
        }

        render_data["index"] = index_info;
        render_data["page_number"] = idx + 1;

        if (augment) {
            augment(render_data, idx, total_pages);
        }

        std::string rendered = env.render(temp, render_data);

        std::filesystem::path root_index = output_dir / "index.html";
        if (idx == 0) {
            std::filesystem::path out = root_index;
            utils::output_file(rendered, out);
        }

        std::string page_number_str = std::to_string(idx + 1);

        std::filesystem::path numbered_path = output_dir / page_number_str / "index.html";
        {
            std::filesystem::path out = numbered_path;
            utils::output_file(rendered, out);
        }

        std::filesystem::path paged_path = output_dir / "pages" / page_number_str / "index.html";
        {
            std::filesystem::path out = paged_path;
            utils::output_file(rendered, out);
        }
    }
}
