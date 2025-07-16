#include "logger.hpp"

void Logger::print_header() {
    constexpr std::string_view title = "~~~   +NUCC Parser++   ~~~";
    constexpr std::string_view title_dec = "~~~   +\033[1;35mNUCC Parser++\033[0m   ~~~";
    constexpr std::string_view version = "[BETA]";
    constexpr std::string_view subheading = "Developed in C++ by Kojo Bailey for all CyberConnect2 XFBIN files!";
    constexpr std::string_view subheading_dec = "Developed in \033[1;34mC++\033[0m by \033[1;31mKojo Bailey\033[0m "
        "for all \033[0;32mCyberConnect2\033[0m \033[1;33mXFBIN\033[0m files!";

    constexpr size_t subheading_length = subheading.size();
    auto padding = [&](std::string_view target) {
        return std::string((subheading_length - target.size()) / 2, ' ');
    };

    std::cout
        << padding(title) << title_dec << "\n"
        << padding(version) << version << "\n"
        << subheading_dec << "\n"
        << std::endl;
}

std::string Logger::level_as_str() {
    switch (current_level) {
        case Level::DEBUG:      return "DEBUG";
        case Level::INFO:       return "INFO";
        case Level::VERBOSE:    return "VERBOSE";
        case Level::WARN:       return "WARN";
        case Level::ERROR:      return "ERROR";
        case Level::FATAL:      return "FATAL";
    }
    return "UNKNOWN";
}

std::string Logger::level_as_colour() {
    switch (current_level) {
        case Level::DEBUG:      return "1;34";  // Light Blue
        case Level::INFO:       return "0";     // Default
        case Level::VERBOSE:    return "1;34";  // Light Blue
        case Level::WARN:       return "1;33";  // Yellow
        case Level::ERROR:      return "1;31";  // Light Red
        case Level::FATAL:      return "0;31";  // Red
    }
    return "UNKNOWN";
}

std::string Logger::file(std::string_view name) {
    return std::format("`\033[1;32m{}\033[0m`", name);
}
std::string Logger::input(std::string_view name) {
    return std::format("'\033[1;35m{}\033[0m'", name);
}

void Logger::start_timer(int index) {
    timers[index].start = std::chrono::high_resolution_clock::now();
}
float Logger::end_timer(int index) {
    timers[index].end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = timers[index].end - timers[index].start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
}