#include <iomanip>
#include "page.hpp"
#include "../utils/utils.hpp"
#include "../Builder/builder.hpp"

Page::Page(const Data& data) 
    : page_data(data) {
}

void Page::render(Config& config) {
    std::string template_name = page_data.get<std::string>("template");
    inja::Environment& env = config.getEnvironment();
    const inja::Template& temp = config.getTemplate(template_name);

    nlohmann::json temp_data = config.getData(DataType::SITE).getJson();
    temp_data["page"] = page_data.getJson();

    // render and output file
    std::string result = env.render(temp, temp_data);
    std::filesystem::path output_path = page_data.get<std::string>("path");
    utils::output_file(result, output_path);
}

void Page::validate(Config& config) {
    if (!page_data.hasKey("title")) {
        page_data.set<std::string>("Untitled Page", "title");
    }

    if (!page_data.hasKey("date")) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
        
        page_data.set<std::string>(oss.str(), "date");
    }

    if (!page_data.hasKey("template")) {
        // TODO: try getting the default template from the theme config
    }

    
    std::time_t timestamp;
    std::tm tm = {};
    std::stringstream ss(page_data.get<std::string>("date"));
    ss >> std::get_time(&tm, "%d-%m-%Y %H:%M");
    timestamp = mktime(&tm);
    
    page_data.set<std::time_t>(timestamp, "timestamp");
}