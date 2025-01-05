#include "main.hpp"

std::string Logger::level_as_str() {
    switch (current_level) {
        case Level::DEBUG:  return "DEBUG";
        case Level::INFO:   return "INFO";
        case Level::WARN:   return "WARN";
        case Level::ERROR:  return "ERROR";
        case Level::FATAL:  return "FATAL";
    }
    return "UNKNOWN";
}

std::string Logger::level_as_colour() {
    switch (current_level) {
        case Level::DEBUG:  return "1;34";
        case Level::INFO:   return "0";
        case Level::WARN:   return "1;33";
        case Level::ERROR:  return "1;31";
        case Level::FATAL:  return "0;31";
    }
    return "UNKNOWN";
}