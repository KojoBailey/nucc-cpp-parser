#include <chrono>
#include <iostream>
#include <format>
#include <regex>

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

    void print_header();

    template<typename... Args>
    void send(Level level, std::format_string<Args...> fmt, Args&&... args) {
        current_level = level;
        log(std::format(fmt, std::forward<Args>(args)...));
    }
    template<typename T>
    void send(Level level, T&& content) {
        current_level = level;
        log(std::forward<T>(content));
    }

    template<typename... Args>
    void debug(Args&&... args) {
        send(Level::DEBUG, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void info(Args&&... args) {
        send(Level::INFO, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void verbose(Args&&... args) {
        send(Level::VERBOSE, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void warn(Args&&... args) {
        send(Level::WARN, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void error(Args&&... args) {
        send(Level::ERROR, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void fatal(Args&&... args) {
        send(Level::FATAL, std::forward<Args>(args)...);
    }

    template<typename... Args>
    std::string get_input(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...);
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    std::string file(std::string_view name);
    std::string input(std::string_view name);

    void start_timer(int index);
    float end_timer(int index);

private:
    Level current_level;
    std::string level_as_str();
    std::string level_as_colour();

    struct Timer {
        std::chrono::high_resolution_clock::time_point start, end;
    };
    std::array<Timer, 2> timers;

    #define LOG_FMT "\033[{}m> [{}] {}\033[0m"

    template<typename T> void log(T&& content) {
        std::string output;
        output = std::regex_replace(content, std::regex("\\[0m"), std::format("[{}m", level_as_colour()));
        std::cout << std::format(LOG_FMT, level_as_colour(), level_as_str(), output) << std::endl;
    }
};
inline Logger logger;