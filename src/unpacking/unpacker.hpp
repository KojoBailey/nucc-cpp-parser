#include "../config.hpp"

#include <nucc/xfbin.hpp>

#include <cc2/binary_serializer.hpp>
#include <cc2/json_serializer.hpp>

#include <nlohmann/json.hpp>
#include <filesystem>

class XFBIN_Unpacker {
public:
    struct Page {
        kojo::logger log{"Page Unpacker", true, true};

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
            std::string filename;
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

        template<typename T>
        void Parse_Data(Chunk& chunk) {
            T binary_data = cc2::binary_serializer<T>::read(chunk.data.data());
            nlohmann::ordered_json json_output = cc2::json_serializer<T>::write(binary_data);
            std::ofstream output(path / std::format("{:03} - {}.json", chunk.index, chunk.data.name()));
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
    kojo::logger log{"XFBIN Unpacker", true, true};

    nucc::xfbin xfbin;
    std::filesystem::path xfbin_path;
    std::filesystem::path unpacked_directory_path;
    nlohmann::ordered_json index_json;

    void Get_Game();
    void Create_Main_Directory();
    void Write_Index_JSON();
    void Create_Page_Directories();
    void Process_Page(Page page);
    void Create_Index_File();
};