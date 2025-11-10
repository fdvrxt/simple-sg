#include <filesystem>
#include <iostream>
#include <fstream>
#include <cwchar>
#include <vector>
#include <md4c-html.h>
#include <inja.hpp>

#include "data/config.hpp"
#include "data/data.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"
#include "utils/debug.hpp"
#include "builder/builder.hpp"

#ifdef DEBUG
#include <chrono>
#endif

int main(int argc, char* argv[]) {
#ifdef DEBUG
    auto start_time = std::chrono::high_resolution_clock::now();
    //argc = 2;
    //argv[1] = const_cast<char*>("C:/Users/Larry/Desktop/ssg-instance/config.json");
#endif

    bool return_value = 0;
    std::filesystem::path config_path;

    try {
        if (argc > 1) {
            config_path = std::filesystem::path(argv[1]);
            LOG_INFO("Using: " << config_path);

            if (!std::filesystem::exists(config_path)) {
                throw std::runtime_error("Specified config file not found");
            }
        } else {
            config_path = std::filesystem::path("config.json");
            if (!std::filesystem::exists(config_path)) {
                throw std::runtime_error("No config file specified and config.json not found in current working directory");
            }
        }

        Config config(config_path);
        Feeder feeder(config);
        Builder builder(feeder);
        
        builder.build();
        LOG_INFO("Building succeeded");
    } catch (const std::exception& e) {
        LOG_ERROR("Building failed: " << e.what());
        return_value = 1;   
    }

#ifdef DEBUG
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Execution time: " << duration.count() << " seconds\n";
#endif
    LOG_INFO("Press any key to exit...");
    getchar();
    return return_value;
}
