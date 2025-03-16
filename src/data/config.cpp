#include "config.hpp"
#include "../utils/utils.hpp"

// factory for site and theme configuration data objects
Data Config::createData(DataType type, const std::filesystem::path& path) {
    try {
        std::ifstream config_file(path);
        if (!config_file.is_open()) {
            std::stringstream ss;
            if (type == DataType::SITE) {
                ss << "Failed to open site configuration file: " << path << std::endl;
            } else {
                ss << "Failed to open theme configuration file: " << path << std::endl;
            }
            throw std::runtime_error(ss.str());
        }

        if (type == DataType::SITE) {
            return Data("site", config_file);
        } else {
            return Data(config_file);
        }
    } catch (const std::exception& e) {
        std::stringstream ss;
        if (type == DataType::SITE) {
            ss << "Error creating Site config from configuration file " << path << ": " << e.what() << std::endl;
        } else {
            ss << "Error creating Theme config from configuration file " << path << ": " << e.what() << std::endl;
        }
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
    validate();
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

void Config::validate() 
{
    // TODO: validate the site and theme configuration data    
}