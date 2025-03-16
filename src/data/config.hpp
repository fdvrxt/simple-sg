#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <filesystem>
#include <inja.hpp>
#include <mutex>
#include "data.hpp"
#include "../utils/debug.hpp"

enum class DataType { 
    SITE, THEME
};

class Config {
private:
    inja::Environment env;
    std::map<std::string, inja::Template> template_map;

    std::filesystem::path site_dir;;
    Data site_data;
    std::filesystem::path theme_dir;
    Data theme_data;

    // factories
    Data                    createData(DataType type, const std::filesystem::path& path);
    std::filesystem::path   createDirectory(const std::string& field);
    std::filesystem::path   getThemeFromSiteData() const;

public:
    Config(const std::filesystem::path& config_path);
    void validate();

    inja::Environment&              getEnvironment() { return env; }
    const inja::Template&           getTemplate(const std::string& template_name);
    Data&                           getData(DataType type) { return (type == DataType::SITE ? site_data : theme_data); }
    const std::filesystem::path&    getSiteDirectory() const { return site_dir; }
    const std::filesystem::path&    getThemeDirectory() const { return theme_dir; }


    // forwarding method for calling Data functions dynamically
    template <typename Func, typename... Args>
    auto forward(DataType type, Func func, Args&&... args) -> decltype((std::declval<Data>().*func)(std::forward<Args>(args)...)) {
        if (type == DataType::SITE) {
            return (site_data.*func)(std::forward<Args>(args)...);
        } else {
           return (theme_data.*func)(std::forward<Args>(args)...);
        }
    }

    // wrappers for calling methods on Data objects
    template <typename T, typename... Keys>
    void set(DataType type, T value, Keys... keys) {
        forward(type, &Data::set<T, Keys...>, value, keys...);
    }

    template <typename T, typename... Keys>
    T get(DataType type, Keys... keys) {
        return forward(type, &Data::get<T, Keys...>, keys...);
    }

    template <typename T, typename... Keys>
    void add(DataType type, T value, Keys... keys) {
        forward(type, &Data::add<T, Keys...>, value, keys...);
    }

    template <typename T, typename... Keys>
    void setIfNotExist(DataType type, T value, Keys... keys) {
        forward(type, &Data::setIfNotExist<T, Keys...>, value, keys...);
    }

    template <typename... Keys>
    bool hasKey(DataType type, Keys... keys) const {
        return forward(type, &Data::hasKey<Keys...>, keys...);
    }
};

#endif
