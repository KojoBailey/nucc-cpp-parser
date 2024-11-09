#include <nucc/xfbin.hpp>
#include <nucc/chunks/binary/asbr.hpp>

#include <regex>

// CONFIGS
static int json_spacing = 2;

// LOGGING
template<typename... Args> void Log(std::format_string<Args...> fmt, Args&&... args) {
    std::cout << "> " << std::format(fmt, std::forward<Args>(args)...) << std::endl;
}
void Log(std::string& str);

// CONTROL
void Unpack_XFBIN(std::filesystem::path& xfbin_path);
void Repack_XFBIN(std::filesystem::path& xfbin_path);