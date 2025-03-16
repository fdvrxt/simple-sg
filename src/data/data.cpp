#include "data.hpp"

Data::Data(const nlohmann::json& data) : data(data) { }

Data::Data(std::ifstream& fs) {
    try {
        data = nlohmann::json::parse(fs);
        if (fs.fail()) {
            throw std::runtime_error("Failed to read from file stream.");
        }
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error parsing JSON from file stream: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

Data::Data(const std::string& str) {
    try {
        data = nlohmann::json::parse(str);
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error parsing JSON string: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

Data::Data(const std::string& key, std::ifstream& fs) {
    try {
        data[key] = nlohmann::json::parse(fs);
        if (fs.fail()) {
            throw std::runtime_error("Failed to read from file stream.");
        }
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error parsing JSON from file stream for key '" << key << "': " << e.what();
        throw std::runtime_error(ss.str());
    }
}
