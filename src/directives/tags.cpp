#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "tags.hpp"
#include "index.hpp"

namespace {
    std::string slugify(const std::string& value)
    {
        std::string slug;
        slug.reserve(value.size());

        bool last_was_hyphen = false;
        for (unsigned char c : value) {
            if (std::isalnum(c)) {
                slug.push_back(static_cast<char>(std::tolower(c)));
                last_was_hyphen = false;
            }
            else if (std::isspace(c) || c == '-' || c == '_' || c == '/') {
                if (!last_was_hyphen && !slug.empty()) {
                    slug.push_back('-');
                    last_was_hyphen = true;
                }
            }
        }

        while (!slug.empty() && slug.back() == '-') {
            slug.pop_back();
        }

        if (slug.empty()) {
            slug = "tag";
        }

        return slug;
    }
}

void Tags::init(Config& config, const nlohmann::json directive)
{
    nlohmann::json data = config.getData().getJson();

    if (!data.contains("site") || !data["site"].contains("pages")) {
        LOG_WARN("No pages available for tags directive");
        return;
    }

    const nlohmann::json& pages = data["site"]["pages"];
    if (!pages.is_array() || pages.empty()) {
        LOG_WARN("Tags directive found no pages to render");
        return;
    }

    std::unordered_map<std::string, nlohmann::json> tags_map;

    for (const auto& page : pages) {
        if (!page.contains("tags") || !page["tags"].is_array()) {
            continue;
        }

        for (const auto& tag_value : page["tags"]) {
            if (!tag_value.is_string()) {
                continue;
            }

            std::string tag_name = tag_value.get<std::string>();
            std::string slug = slugify(tag_name);

            nlohmann::json& tag_entry = tags_map[slug];
            if (tag_entry.is_null()) {
                tag_entry = nlohmann::json::object();
                tag_entry["name"] = tag_name;
                tag_entry["slug"] = slug;
                tag_entry["pages"] = nlohmann::json::array();
            }

            tag_entry["pages"].push_back(page);
        }
    }

    if (tags_map.empty()) {
        LOG_WARN("Tags directive found no tags to process");
        return;
    }

    std::vector<nlohmann::json> tag_collection;
    tag_collection.reserve(tags_map.size());
    for (auto& [slug, entry] : tags_map) {
        entry["count"] = entry["pages"].size();
        tag_collection.push_back(entry);
    }

    std::sort(
        tag_collection.begin(),
        tag_collection.end(),
        [](const nlohmann::json& a, const nlohmann::json& b) {
            return a["name"].get<std::string>() < b["name"].get<std::string>();
        }
    );

    config.getData().set<nlohmann::json>(tag_collection, "site", "tags");
    data["site"]["tags"] = tag_collection;

    const inja::Template& temp = config.getTemplate(directive["name"]);
    std::filesystem::path tags_output_dir = config.getSiteDirectory() / "output" / "tags";

    int directive_count = -1;
    if (directive.contains("count")) {
        directive_count = directive["count"].get<int>();
        if (directive_count <= 0) {
            throw std::runtime_error("Tags directive requires a positive 'count' value when provided");
        }
    }

    for (const auto& tag_entry : tag_collection) {
        const nlohmann::json& tag_pages = tag_entry["pages"];
        if (!tag_pages.is_array() || tag_pages.empty()) {
            continue;
        }

        int count = directive_count > 0 ? directive_count : static_cast<int>(tag_pages.size());

        std::filesystem::path tag_output_dir = tags_output_dir / tag_entry["slug"].get<std::string>();

        Index::render_paginated(
            config,
            temp,
            data,
            tag_pages,
            count,
            tag_output_dir,
            [&tag_entry, &tag_collection](nlohmann::json& render_data, std::size_t, std::size_t) {
                render_data["tag"] = tag_entry;
                render_data["tag"]["pages"] = render_data["pages"];
                render_data["tags"] = tag_collection;
            }
        );
    }
}

void Tags::render()
{

}
