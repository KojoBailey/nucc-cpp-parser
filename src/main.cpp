#include "main.hpp"

int main(int argc, char* argv[]) {
    std::filesystem::path xfbin_path;
    if (argc > 1) {
        config.load("settings.json");

        xfbin_path = argv[1];
        if (xfbin_path.extension() == ".xfbin") {
            Unpack_XFBIN(xfbin_path);
        } else if (std::filesystem::is_directory(xfbin_path)) {
            Repack_XFBIN(xfbin_path);
        } else {
            logger.send(Logger::Level::FATAL, "Invalid input file.");
            logger.send(Logger::Level::INFO, "Input must either be:");
            logger.send(Logger::Level::INFO, "- An XFBIN with file extension `.xfbin`.");
            logger.send(Logger::Level::INFO, "- A directory containing an unpacked XFBIN from this tool.");
            return 0;
        }
    } else {
        logger.send(Logger::Level::INFO, "Drag an XFBIN file or directory onto this executable to get started!");
        return 0;
    }
}