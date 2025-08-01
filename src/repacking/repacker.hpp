#include "../config.hpp"

#include <nucc/xfbin.hpp>

#include <cc2/binary_serializer.hpp>
#include <cc2/json_serializer.hpp>

#include <nlohmann/json.hpp>
#include <filesystem>

class XFBIN_Repacker {
public:
    struct Page {
        kojo::logger log{"Page Repacker", true, true};

        size_t index;
        std::string name;
        std::filesystem::path path;
        nlohmann::ordered_json json;

        struct Chunk {
            const nucc::chunk& data;
            size_t index;
            nlohmann::ordered_json json;
            std::string filename;
            std::string filename_fmt;

            Chunk(const nucc::chunk& _data, size_t _index)
            : data(_data), index(_index) {}

            void Write_JSON();
        };

        Page(std::filesystem::path& _path) : path(_path) {}

        void Repack();
        void Read_JSON();
    };

    explicit XFBIN_Repacker(const std::filesystem::path& _xfbin_path);
    XFBIN_Repacker(const XFBIN_Repacker& copy) = delete;
    XFBIN_Repacker& operator=(const XFBIN_Repacker& copy) = delete;

    void Repack();

private:
    kojo::logger log{"XFBIN Repacker", true, true};

    nucc::xfbin xfbin;
    std::filesystem::path xfbin_path;
    std::filesystem::path unpacked_directory_path;
    nlohmann::ordered_json index_json;

    void Read_Index_JSON();
    void Get_Game();
    void Read_Pages();
    void Write_XFBIN();
};