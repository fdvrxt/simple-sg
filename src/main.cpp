#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "data/config.hpp"
#include "data/data.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"
#include "builder/builder.hpp"
#include "utils/debug.hpp"

namespace {
    constexpr unsigned short DEFAULT_SERVER_PORT = 5500;
    const std::string LIVE_RELOAD_SNIPPET = R"(
<script>
(function () {
    const reloadPath = "/__simple-sg_reload__";
    let lastToken = null;
    let consecutiveFailures = 0;

    async function poll() {
        try {
            const response = await fetch(reloadPath, {
                cache: "no-store",
                headers: {
                    "Cache-Control": "no-cache",
                },
            });

            if (!response.ok) {
                throw new Error(`unexpected status ${response.status}`);
            }

            const token = (await response.text()).trim();
            if (token) {
                if (lastToken === null) {
                    lastToken = token;
                } else if (lastToken !== token) {
                    lastToken = token;
                    window.location.reload();
                    return;
                }
            }

            consecutiveFailures = 0;
        } catch (error) {
            consecutiveFailures += 1;
            if (consecutiveFailures % 5 === 0) {
                console.warn("simple-sg live reload: retrying after errors", error);
            }
        }

        setTimeout(poll, 100);
    }

    document.addEventListener("visibilitychange", () => {
        if (document.visibilityState === "visible") {
            poll();
        }
    });

    poll();
})();
</script>
)";

    struct BuildResult {
        std::filesystem::path site_dir;
        std::filesystem::path output_dir;
    };

    class DirectoryWatcher {
    public:
        explicit DirectoryWatcher(std::filesystem::path directory)
            : directory_path(std::move(directory)),
            snapshot(capture_state()) {
        }

        bool has_changes() {
            auto current_state = capture_state();
            bool changed = false;

            for (const auto& [path, timestamp] : current_state) {
                auto it = snapshot.find(path);
                if (it == snapshot.end() || it->second != timestamp) {
                    changed = true;
                    break;
                }
            }

            if (!changed) {
                for (const auto& [path, _] : snapshot) {
                    if (current_state.find(path) == current_state.end()) {
                        changed = true;
                        break;
                    }
                }
            }

            if (changed) {
                snapshot = std::move(current_state);
            }

            return changed;
        }

    private:
        std::filesystem::path directory_path;
        std::unordered_map<std::string, std::filesystem::file_time_type> snapshot;

        std::unordered_map<std::string, std::filesystem::file_time_type> capture_state() const {
            std::unordered_map<std::string, std::filesystem::file_time_type> state;

            if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path)) {
                throw std::runtime_error("Content directory not found or inaccessible: " + directory_path.string());
            }

            std::error_code ec;
            std::filesystem::recursive_directory_iterator iterator(
                directory_path,
                std::filesystem::directory_options::skip_permission_denied,
                ec
            );

            for (auto it = iterator; it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
                if (ec) {
                    LOG_WARN("Unable to access " << it->path() << ": " << ec.message());
                    ec.clear();
                    continue;
                }

                if (!it->is_regular_file(ec)) {
                    ec.clear();
                    continue;
                }

                auto write_time = std::filesystem::last_write_time(it->path(), ec);
                if (ec) {
                    LOG_WARN("Unable to query last write time for " << it->path() << ": " << ec.message());
                    ec.clear();
                    continue;
                }

                state[it->path().string()] = write_time;
            }

            return state;
        }
    };

    void update_reload_token(const std::filesystem::path& output_dir) {
        std::error_code ec;
        std::filesystem::create_directories(output_dir, ec);
        if (ec) {
            LOG_WARN("Failed to create output directory for live reload token: " << output_dir << ". Error: " << ec.message());
            return;
        }

        auto token_path = output_dir / "__simple-sg_reload__";
        auto now = std::chrono::system_clock::now().time_since_epoch();
        auto token_value = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());

        std::filesystem::path writable_path = token_path;
        if (!utils::output_file(token_value, writable_path)) {
            LOG_WARN("Failed to update live reload token file: " << token_path);
        }
    }

    BuildResult build_site(const std::filesystem::path& config_path, bool enable_live_reload) {
        Config config(config_path);
        Feeder feeder(config);
        Builder builder(feeder, enable_live_reload ? LIVE_RELOAD_SNIPPET : "");

        builder.build();

        std::filesystem::path site_dir = config.getSiteDirectory();
        std::filesystem::path output_dir = site_dir / "output";

        if (enable_live_reload) {
            update_reload_token(output_dir);
        }

        return { site_dir, output_dir };
    }

    std::string detect_python_command() {
        std::vector<std::string> candidates;
#ifdef _WIN32
        candidates = { "py -3", "py", "python", "python3" };
        const char* redirect = " >nul 2>&1";
#else
        candidates = { "python3", "python" };
        const char* redirect = " >/dev/null 2>&1";
#endif

        for (const auto& candidate : candidates) {
            std::string command = candidate + " --version" + redirect;
            if (std::system(command.c_str()) == 0) {
                return candidate;
            }
        }

        throw std::runtime_error("Python interpreter not found. Please install Python 3 to use the live server.");
    }

    std::string build_server_command(const std::string& python_command, const std::filesystem::path& output_dir, unsigned short port) {
        std::ostringstream ss;
#ifdef _WIN32
        ss << "cd /d \"" << output_dir.string() << "\" && " << python_command << " -m http.server " << port;
#else
        ss << "cd \"" << output_dir.string() << "\" && " << python_command << " -m http.server " << port;
#endif
        return ss.str();
    }

    int run_server(const std::filesystem::path& config_path) {
        LOG_INFO("Starting simple-sg live server");

        BuildResult initial_build = build_site(config_path, true);
        LOG_INFO("Initial build complete. Output directory: " << initial_build.output_dir);

        DirectoryWatcher watcher(initial_build.site_dir / "content");
        std::atomic<bool> keep_running(true);

        std::thread watcher_thread([&]() {
            while (keep_running.load()) {
                try {
                    if (watcher.has_changes()) {
                        LOG_INFO("Detected changes in content directory. Rebuilding...");
                        try {
                            build_site(config_path, true);
                            LOG_INFO("Rebuild complete");
                        }
                        catch (const std::exception& build_error) {
                            LOG_ERROR("Rebuild failed: " << build_error.what());
                        }
                    }
                }
                catch (const std::exception& e) {
                    LOG_ERROR("Watcher error: " << e.what());
                }

                for (int i = 0; i < 10 && keep_running.load(); ++i) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            });

        std::string python_command = detect_python_command();
        std::string server_command = build_server_command(python_command, initial_build.output_dir, DEFAULT_SERVER_PORT);

        LOG_INFO("Serving \"" << initial_build.output_dir << "\" on http://localhost:" << DEFAULT_SERVER_PORT);
        LOG_INFO("Press Ctrl+C to stop the server");

        int server_status = std::system(server_command.c_str());
        if (server_status != 0) {
            LOG_WARN("Python HTTP server exited with status code: " << server_status);
        }

        keep_running.store(false);
        if (watcher_thread.joinable()) {
            watcher_thread.join();
        }

        return server_status == 0 ? 0 : 1;
    }
}

int main(int argc, char* argv[]) {
    bool server_mode = false;
    std::filesystem::path config_path = "config.json";

    try {
        if (argc > 1) {
            std::string first_argument(argv[1]);
            if (first_argument == "server") {
                server_mode = true;
                if (argc > 2) {
                    config_path = argv[2];
                }
            }
            else {
                config_path = first_argument;
            }
        }

        if (!std::filesystem::exists(config_path)) {
            std::ostringstream ss;
            ss << "Configuration file not found: " << config_path;
            throw std::runtime_error(ss.str());
        }

        if (server_mode) {
            return run_server(config_path);
        }

        BuildResult build = build_site(config_path, false);
        LOG_INFO("Building succeeded. Output directory: " << build.output_dir);

#ifdef DEBUG
        LOG_INFO("Press any key to exit...");
        getchar();
#endif

        return 0;
    }
    catch (const std::exception& e) {
        LOG_ERROR("simple-sg failed: " << e.what());
        return 1;
    }
}