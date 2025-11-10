#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <filesystem>
#include <inja.hpp>
#include <mutex>
#include "data.hpp"
#include "../utils/debug.hpp"


class Config {
private:
    inja::Environment env;
    std::map<std::string, inja::Template> template_map;

    // initialization list order
    std::filesystem::path site_dir;
    Data data;
    std::filesystem::path theme_dir;

    std::filesystem::path   siteDirFactory(const std::filesystem::path& path);
    std::filesystem::path   themeDirFactory() const;
    Data                    dataFactory(const std::filesystem::path& path);

public:
    Config(const std::filesystem::path& config_path);
    void validate_theme_config();
    void validate_site_config();

    inja::Environment&              getEnvironment() { return env; }
    const inja::Template&           getTemplate(const std::string& template_name);
    
    const std::filesystem::path&    getSiteDirectory() const { return site_dir; }
    const std::filesystem::path&    getThemeDirectory() const { return theme_dir; }
    Data&                           getData() { return data; }
    std::vector<nlohmann::json>     get_directives();

    // default config values
    static constexpr const char* DEFAULT_SITE_TITLE = "Site";
};
#endif
