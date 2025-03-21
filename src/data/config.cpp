#include "config.hpp"
#include "../utils/utils.hpp"
#include "../utils/logger.hpp"

// factory for site and theme configuration data objects
Data Config::createData(DataType type, const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Configuration file not found: " + path.string());
        }

        std::ifstream config_file(path);
        if (!config_file) {
            throw std::runtime_error("Failed to open configuration file: " + path.string());
        }

        // site and page data is combined before rendering, it's prefixed with `site` to avoid collisions
        return (type == DataType::SITE) ? Data("site", config_file) : Data(config_file);

    } catch (const std::exception& e) {
        std::ostringstream ss;
        ss << "Error reading " 
           << ((type == DataType::SITE) ? "site" : "theme")  << " "
           << "configuration file"
           << ": " << e.what();
        throw std::runtime_error(ss.str());
    }
}

std::filesystem::path Config::getThemeFromSiteData() const {
    try {
        return site_dir / "themes" / site_data.get<std::string>("site", "theme");
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error getting theme name from config.json: " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

Config::Config(const std::filesystem::path& path) : 
    site_dir    (path.parent_path()),
    site_data   (createData(DataType::SITE, path)),
    theme_dir   (getThemeFromSiteData()),
    theme_data  (createData(DataType::THEME, theme_dir / "config.json"))
{
    try {
        validate_theme_config();
        validate_site_config();
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error validating config.json: " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

const inja::Template& Config::getTemplate(const std::string& template_name) {
    static std::mutex template_mutex;
    std::lock_guard<std::mutex> lock(template_mutex);

    if (template_map.find(template_name) != template_map.end()) {
        return template_map[template_name];
    }

    try {
        std::filesystem::path template_path = theme_dir / theme_data.get<std::string>("templates", template_name);

        if (!std::filesystem::exists(template_path)) {
            std::stringstream ss;
            ss << "Template file not found: " << template_path << std::endl;
            throw std::runtime_error(ss.str());
        }

        template_map[template_name] = env.parse_template(template_path.string());
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error while retrieving template '" << template_name << "': " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }

    return template_map[template_name];
}

void Config::validate_theme_config() {
    // there might be reasons to build without templates?
    if (!theme_data.hasKey("templates")) {
        LOG_WARN("No templates found in theme config.json");
    }
    
    if (!theme_data.hasKey("default")) {
        LOG_WARN("No default template specified in theme config.json");
    }

    if (!theme_data.hasKey("assets-directory")) {
        LOG_WARN("No assets directory found in theme config.json");
    }
}

void Config::validate_site_config() {
    if (!site_data.hasKey("site", "url")) {
        throw std::runtime_error("No url found in site config.json");
    }

    if (!site_data.hasKey("site", "title")) {
        LOG_WARN("No title found in site config.json. Defaulting to: " << DEFAULT_SITE_TITLE);
        site_data.set<std::string>(DEFAULT_SITE_TITLE, "site", "title");
    }

    // a better idea would be to just pick one of the available themes and warn
    // but we will enforce this for now
    if (!site_data.hasKey("site", "theme")) {
        throw std::runtime_error("No theme specified in site config.json");
    }
}