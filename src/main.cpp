#include "config.hpp"
#include "unpacking/unpacker.hpp"
// #include "unpacking/repacker.hpp"

constexpr std::string_view XFBIN_EXTENSION = ".xfbin";
constexpr std::string_view SETTINGS_PATH = "settings.json";

void print_header() {
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

int main(int argc, char* argv[]) {
    print_header();
    
    kojo::logger log{"NUCC++ Parser"};

    if (argc <= 1) {
        log.fatal(
            kojo::logger::status::null_file,
            "No input file(s).",
            "Drag an XFBIN file or directory onto this executable to get started!"
        );
        return 1;
    }

    const std::filesystem::path xfbin_path = argv[1];

    if (!std::filesystem::exists(xfbin_path)) {
        log.fatal(
            kojo::logger::status::null_file,
            "Specified path does not exist.",
            "Ensure the path you input exists and is typed correctly."
        );
        return 1;
    }

    if (std::filesystem::is_regular_file(xfbin_path) && xfbin_path.extension() == XFBIN_EXTENSION) {
        config.load(SETTINGS_PATH);
        XFBIN_Unpacker unpacker{xfbin_path};
        unpacker.Unpack();
    } else if (std::filesystem::is_directory(xfbin_path)) {
        // repack_xfbin(xfbin_path);
    } else {    
        log.fatal(
            kojo::logger::status::bad_value,
            "Invalid input file.",
            std::format("Input must either be:"
                "\n- An XFBIN with file extension {}."
                "\n- A directory containing an unpacked XFBIN from this tool.", XFBIN_EXTENSION)
        );
        return 1;
    }
    
    return 0;
}