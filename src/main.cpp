#include <nucc/xfbin.hpp>
#include <nucc/chunks/binary/asbr.hpp>
#include <nucc/hash.hpp>

template<typename... Args> void log(std::format_string<Args...> fmt, Args&&... args) {
    std::cout << "> " << std::format(fmt, std::forward<Args>(args)...) << std::endl;
}
void log(std::string& str) {
    std::cout << "> " << str << std::endl;
}

int main(int argc, char* argv[]) {
    std::filesystem::path xfbin_path;
    if (argc > 1) {
        xfbin_path = argv[1];
        if (xfbin_path.extension() != ".xfbin") {
            log("File must be an XFBIN with file extension `.xfbin`.");
            return 0;
        }
    } else {
        log("Drag an XFBIN onto this executable to get started!");
        return 0;
    }

    nucc::XFBIN xfbin{xfbin_path};
    nucc::Binary buffer{xfbin.fetch<nucc::Binary>("messageInfo")};
    nucc::ASBR::messageInfo message_info{buffer.data()};
    log(message_info.entries[nucc::hash("5grn01_btlst_00_5bct01")].message);
}