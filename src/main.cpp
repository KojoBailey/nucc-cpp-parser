#include "main.hpp"

void Log(std::string& str) {
    std::cout << "> " << str << std::endl;
}

int main(int argc, char* argv[]) {
    std::filesystem::path xfbin_path;
    if (argc > 1) {
        xfbin_path = argv[1];
        if (xfbin_path.extension() == ".xfbin") {
            Unpack_XFBIN(xfbin_path);
        } else if (std::filesystem::is_directory(xfbin_path)) {
            Repack_XFBIN(xfbin_path);
        } else {
            Log("Input must either be:");
            Log("\tAn XFBIN with file extension `.xfbin`.");
            Log("\tA directory containing an unpacked XFBIN from this tool.");
            return 0;
        }
    } else {
        Log("Drag an XFBIN file or directory onto this executable to get started!");
        return 0;
    }
}