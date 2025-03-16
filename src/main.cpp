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
#include "utils/debug.hpp"
#include "builder/builder.hpp"

int main(int argc, char* argv[]) {
    bool return_value = 0;
    std::filesystem::path config_path;

    // vscode launch.json args do not seem to work for some reason. temporary workaround
#ifdef DEBUG
    argc = 2;
    argv[1] = "C:\\Users\\big pc\\Desktop\\ssg-instance\\config.json";
#endif

    try {
        if (argc > 1) {
            config_path = std::filesystem::path(argv[1]);
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
    } catch (const std::exception& e) {
        std::cerr << "Building failed: " << e.what() << std::endl;
        return_value = 1;   
    }

    getchar();
    return return_value;
}
