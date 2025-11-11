#include <algorithm>
#include <cctype>
#include <iomanip>
#include "page.hpp"
#include "../utils/utils.hpp"
#include "../builder/builder.hpp"
#include "../directives/directive.hpp"

Page::Page(const Data& data)
    : page_data(data)
{
}

void Page::render(Config& config, const std::string& live_reload_snippet) {
    std::string template_name = page_data.get<std::string>("template");
    inja::Environment& env = config.getEnvironment();
    const inja::Template& temp = config.getTemplate(template_name);

    nlohmann::json temp_data = config.getData().getJson();
    temp_data["page"] = page_data.getJson();

    std::string result = env.render(temp, temp_data);

    if (!live_reload_snippet.empty()) {
        std::string lowered = result;
        std::transform(
            lowered.begin(),
            lowered.end(),
            lowered.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );
        std::string body_tag = "</body>";
        std::size_t pos = lowered.rfind(body_tag);

        if (pos != std::string::npos) {
            result.insert(pos, live_reload_snippet);
        }
        else {
            result.append(live_reload_snippet);
        }
    }
    std::filesystem::path output_path = page_data.get<std::string>("path");

    if (utils::output_file(result, output_path)) {
        LOG_INFO("Succefully outputted file: " << output_path);
    }
    else {
        LOG_ERROR("Failed outputting file: " << output_path);
    }
}

void Page::validate(Config& config) {
    if (!page_data.hasKey("title")) {
        LOG_WARN("No title found in page frontmatter. Defaulting to: " << DEFAULT_PAGE_TITLE);
        page_data.set<std::string>(DEFAULT_PAGE_TITLE, "title");
    }

    // TODO: generate description from content instead
    if (!page_data.hasKey("description")) {
        LOG_WARN("No description found in page frontmatter. Defaulting to: " << DEFAULT_PAGE_DESCRIPTION);
        page_data.set<std::string>(DEFAULT_PAGE_DESCRIPTION, "description");
    }

    if (!page_data.hasKey("date")) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
        
        page_data.set<std::string>(oss.str(), "date");
    }

    if (!page_data.hasKey("indexable")) {
        page_data.set<bool>(true, "indexable");
    }

    if (!page_data.hasKey("show_description")) {
        page_data.set<bool>(false, "show_description");
    }

    if (!page_data.hasKey("template")) {
        if (config.getData().hasKey("theme", "default")) {
            page_data.set<std::string>(
                config.getData().get<std::string>("theme", "default"),
                "template"
            );
        } else {
            throw std::runtime_error("No template provided in frontmatter and theme has no default template.");
        }
    }
    
    std::time_t timestamp;
    std::tm tm = {};
    std::stringstream ss(page_data.get<std::string>("date"));
    ss >> std::get_time(&tm, "%d-%m-%Y %H:%M");
    timestamp = mktime(&tm);
    
    page_data.set<std::time_t>(timestamp, "timestamp");
}
