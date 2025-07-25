#ifndef DATA_HPP_
#define DATA_HPP_

#include <nlohmann/json.hpp>
#include <inja.hpp>
#include "../utils/debug.hpp"

class Data {
private:
    nlohmann::json data;
    
public:
    Data() = default;
    Data(std::ifstream& fs);
    Data(const std::string& str, std::ifstream& fs);
    Data(const std::string& data);
    Data(const nlohmann::json& data);

    template<typename T, typename... Keys>
    void set(T value, Keys... keys) {
        nlohmann::json* current = &data;
        try {
            ((current = &((*current)[keys])), ...);
            if constexpr (std::is_same_v<T, Data>) {
                *current = value.getJson();
            } else {
                *current = value;
            }
        } catch (const nlohmann::json::exception& e) {
            std::stringstream ss;
            ss << "Error setting value: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    template<typename T, typename... Keys>
    T get(Keys... keys) const {
        const nlohmann::json* current = &data;
        try {
            ((current = &((*current).at(keys))), ...);
            if constexpr (std::is_same_v<T, Data>) {
                return Data(*current);
            } else {
                return current->get<T>();
            }
        } catch (const nlohmann::json::out_of_range& e) {
            std::stringstream ss;
            ss << "Key not found: " << e.what();
            throw std::runtime_error(ss.str());
        } catch (const nlohmann::json::type_error& e) {
            std::stringstream ss;
            ss << "Type error during get: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }

    // add to array
    template <typename T, typename... Keys>
    void add(T value, Keys... keys) {
        nlohmann::json* current = &data;
        try {
            ((current = &((*current)[keys])), ...);
            if (!current->is_array()) {
                *current = nlohmann::json::array();
            }
            
            if constexpr (std::is_same_v<T, Data>) {
                current->push_back(value.getJson());
            } else {
                current->push_back(value);
            }
        } catch (const nlohmann::json::exception& e) {
            std::stringstream ss;
            ss << "Error adding value: " << e.what();
            throw std::runtime_error(ss.str());
        }
    }
   
    template<typename T, typename... Keys>
    void setIfNotExist(T value, Keys... keys) {
        try {
            get<T>(keys...);
        } catch (const std::exception& e) {
            set(value, keys...);
        }
    }
    
    template <typename... Keys>
    bool hasKey(Keys... keys) {
        const nlohmann::json* current = &data;
    
        return (... && (current->is_object() &&
                        (current = (current->find(keys) != current->end() ? &current->at(keys) : nullptr))));
    }

    void extend(Data d, const std::string& key) {
        data[key] = d.getJson();
    }
    
    nlohmann::json getJson() const {
        return data;
    }
};

#endif
