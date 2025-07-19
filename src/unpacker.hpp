#include "config.hpp"

#include <nucc/xfbin_new.hpp>
#include <nucc/chunks/binary/binary_data_new.hpp>
#include <nucc/chunks/binary/json_serializable.hpp>

#include <nlohmann/json.hpp>
#include <filesystem>

class XFBIN_Unpacker {
public:
    struct Page {
        XFBIN_Unpacker* xfbin_unpacker;
        const nucc::page& data;
        size_t index;
        std::string name;
        std::filesystem::path path;
        nlohmann::ordered_json json;

        struct Chunk {
            const nucc::chunk& data;
            size_t index;
            nlohmann::ordered_json json;
            std::string filename_fmt;

            Chunk(const nucc::chunk& _data, size_t _index)
            : data(_data), index(_index) {}

            void Write_JSON();
        };

        Page(XFBIN_Unpacker* _xfbin_unpacker, const nucc::page& _data, size_t _index)
        : xfbin_unpacker(_xfbin_unpacker), data(_data), index(_index) {}

        void Write_Global_JSON();
        void Create_Directory();
        void Process_Chunks();
        void Process_Chunk(Chunk chunk);
        void Write_File();

        template<std::derived_from<nucc::binary_data> T>
        void Parse_Data(Chunk& chunk) {
            T binary_data;
            binary_data.read(chunk.data.data());
            nlohmann::ordered_json json_output = nucc::json_serializer<T>::write(binary_data);
            std::ofstream output(path / std::format("{}.json", chunk.data.path()));
            output << json_output.dump(config.json_spacing);
        }

        void Handle_Chunk_Null(Chunk& chunk);
        void Handle_Chunk_Binary(Chunk& chunk);

        bool Handle_ASBR(Chunk& chunk);
        void Dump_Binary(Chunk& chunk);
    };

    explicit XFBIN_Unpacker(const std::filesystem::path& _xfbin_path);
    XFBIN_Unpacker(const XFBIN_Unpacker& copy) = delete;
    XFBIN_Unpacker& operator=(const XFBIN_Unpacker& copy) = delete;

    void Unpack();

private:
    void Get_Game();
    void Create_Main_Directory();
    void Write_Index_JSON();
    void Create_Page_Directories();
    void Process_Page(Page page);
    void Create_Index_File();

    nucc::xfbin xfbin;
    std::filesystem::path xfbin_path;
    std::filesystem::path unpacked_directory_path;
    nlohmann::ordered_json index_json;
};