#include "config.hpp"
#include "../utils/utils.hpp"
#include "../utils/logger.hpp"

Config::Config(const std::filesystem::path& path) : 
    site_dir    (siteDirFactory(path)),
    data        (dataFactory(path)),
    theme_dir   (themeDirFactory())
{
    try {
        std::ifstream fs(theme_dir / "config.json");
        Data theme(fs);
        fs.close();
        data.extend(theme, "theme");

        validate_site_config();
        validate_theme_config();
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error validating config.json: " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

std::filesystem::path Config::siteDirFactory(const std::filesystem::path& path) {
    return path.parent_path();
}

Data Config::dataFactory(const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Configuration file not found: " + path.string());
        }

        std::ifstream config_file_stream(path);
        if (!config_file_stream) {
            throw std::runtime_error("Failed to open configuration file: " + path.string());
        }

        return Data("site", config_file_stream);
    } catch (const std::exception& e) {
        std::ostringstream ss;
        ss << "Error reading site configuration file"
           << ": " << e.what();
        throw std::runtime_error(ss.str());
    }
}

std::filesystem::path Config::themeDirFactory() const {
    try {
        return site_dir / "themes" / data.get<std::string>("site", "theme");
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error getting theme name from config.json: " << e.what() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void Config::validate_site_config() {
    if (!data.hasKey("site", "url")) {
        throw std::runtime_error("No url found in site config.json");
    }

    if (!data.hasKey("site", "title")) {
        LOG_WARN("No title found in site config.json. Defaulting to: " << DEFAULT_SITE_TITLE);
        data.set<std::string>(DEFAULT_SITE_TITLE, "site", "title");
    }

    // a better idea would be to just pick one of the available themes and warn
    // but we will enforce this for now
    if (!data.hasKey("site", "theme")) {
        throw std::runtime_error("No theme specified in site config.json");
    }
}

void Config::validate_theme_config() {
    if (!data.hasKey("theme", "templates")) {
        throw std::runtime_error("No templates found in theme configuration");
    }
    
    if (!data.hasKey("theme", "default")) {
        LOG_WARN("No default template specified in theme configuration");
    }

    if (!data.hasKey("theme", "assets-directory")) {
        LOG_WARN("No assets directory found in theme configuration");
    }
}

const inja::Template& Config::getTemplate(const std::string& template_name) {
    static std::mutex template_mutex;
    std::lock_guard<std::mutex> lock(template_mutex);

    if (template_map.find(template_name) != template_map.end()) {
        return template_map[template_name];
    }

    try {
        std::filesystem::path template_path = theme_dir / data.get<std::string>("theme", "templates", template_name);

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
