#include <nucc/xfbin_new.hpp>

#include <nlohmann/json.hpp>
#include <filesystem>

class XFBIN_Unpacker {
public:
    struct Page {
        XFBIN_Unpacker* xfbin_unpacker;
        const nucc::Page& data;
        size_t index;
        std::string name;
        std::filesystem::path path;
        nlohmann::ordered_json json;

        struct Chunk {
            const nucc::Chunk& data;
            size_t index;

            Chunk(const nucc::Chunk& _data, size_t _index)
            : data(_data), index(_index) {}
        };

        Page(XFBIN_Unpacker* _xfbin_unpacker, const nucc::Page& _data, size_t _index)
        : xfbin_unpacker(_xfbin_unpacker), data(_data), index(_index) {}

        void write_global_json();
        void create_directory();
        void process_chunks();
        void process_chunk(Chunk chunk);
        void write_file();
    };

    explicit XFBIN_Unpacker(const std::filesystem::path& _xfbin_path);
    XFBIN_Unpacker(const XFBIN_Unpacker& copy) = delete;
    XFBIN_Unpacker& operator=(const XFBIN_Unpacker& copy) = delete;

    void unpack();

private:
    void get_game();
    void create_main_directory();
    void write_index_json();
    void create_page_directories();
    void process_page(Page page);
    void create_index_file();

    nucc::XFBIN xfbin;
    std::filesystem::path xfbin_path;
    std::filesystem::path unpacked_directory_path;
    nlohmann::ordered_json index_json;
};