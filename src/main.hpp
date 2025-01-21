#include <nucc/xfbin.hpp>
#include <nucc/chunks/binary/asbr.hpp>
#include <nucc/chunks/binary/eohps4.hpp>
#include <nucc/chunks/binary/eohps3.hpp>
#include <nucc/chunks/binary/asb.hpp>

#include <array>
#include <chrono>
#include <regex>

// UTILS
inline std::string str_lowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return str;
}

// CONFIGS
class Config {
public:
    nlohmann::ordered_json json;
    
    nucc::Game game{nucc::Game::UNKNOWN};
    int json_spacing{2};

    void generate(std::filesystem::path path);
    void load(std::filesystem::path path);
private:
    void update(std::filesystem::path path);
};
inline Config config;

// LOGGING
class Logger {
public:
    enum class Level {
        DEBUG,      // Only show in debug builds.
        INFO,       // Standard log.
        VERBOSE,    // More info than necessary.
        WARN,       // Make a recommendation or non-crucial alert.
        ERROR,      // Announce a partial, non-fatal error.
        FATAL       // Announce a fatal error, terminating the program.
    };

    void header();

    template<typename... Args> void send(Level level, std::format_string<Args...> fmt, Args&&... args) {
        current_level = level;
        log_lit(std::format(fmt, std::forward<Args>(args)...));
    }
    template<typename T> void send(Level level, T& content) {
        current_level = level;
        log_ref(content);
    }
    template<typename T> void send(Level level, T&& content) {
        current_level = level;
        log_lit(content);
    }

    template<typename... Args> std::string get_input(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...);
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    std::string file(std::string name);
    std::string input(std::string name);

    void timer_start(int index);
    float timer_end(int index);

private:
    Level current_level;
    std::string level_as_str();
    std::string level_as_colour();

    struct Timer {
        std::chrono::high_resolution_clock::time_point start, end;
    };
    std::array<Timer, 2> timers;

    #define LOG_FMT "\033[{}m> [{}] {}\033[0m"

    template<typename T> void log_ref(T& content) {
        std::string output;
        output = std::regex_replace(content, std::regex("\\[0m"), std::format("[{}m", level_as_colour()));
        std::cout << std::format(LOG_FMT, level_as_colour(), level_as_str(), output) << std::endl;
    }
    template<typename T> void log_lit(T&& content) {
        std::string output;
        output = std::regex_replace(content, std::regex("\\[0m"), std::format("[{}m", level_as_colour()));
        std::cout << std::format(LOG_FMT, level_as_colour(), level_as_str(), output) << std::endl;
    }
};
inline Logger logger;

// CONTROL
void Unpack_XFBIN(std::filesystem::path& xfbin_path);
void Repack_XFBIN(std::filesystem::path& xfbin_path);