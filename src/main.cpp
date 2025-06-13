#include "main.hpp"

constexpr std::string_view XFBIN_EXTENSION = ".xfbin";
constexpr std::string_view SETTINGS_PATH = "settings.json";

int main(int argc, char* argv[]) {
    logger.print_header();

    if (argc <= 1) {
        logger.fatal("No input file(s).");
        logger.info("Drag an XFBIN file or directory onto this executable to get started!");
        return 1;
    }

    const std::filesystem::path xfbin_path = argv[1];

    if (!std::filesystem::exists(xfbin_path)) {
        logger.fatal("Specified path does not exist.");
        logger.info("Ensure the path you input exists and is typed correctly.");
        return 1;
    }

    if (std::filesystem::is_regular_file(xfbin_path) && xfbin_path.extension() == XFBIN_EXTENSION) {
        config.load(SETTINGS_PATH);
        XFBIN_Unpacker unpacker{xfbin_path};
        unpacker.unpack();
    } else if (std::filesystem::is_directory(xfbin_path)) {
        repack_xfbin(xfbin_path);
    } else {    
        logger.fatal("Invalid input file.");
        logger.info("Input must either be:");
        logger.info("- An XFBIN with file extension {}.", logger.file(XFBIN_EXTENSION));
        logger.info("- A directory containing an unpacked XFBIN from this tool.");
        return 1;
    }
    
    return 0;
}