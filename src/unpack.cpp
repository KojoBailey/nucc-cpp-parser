#include "unpacker.hpp"

#include "logger.hpp"
#include "config.hpp"

XFBIN_Unpacker::XFBIN_Unpacker(const std::filesystem::path& _xfbin_path) {
    xfbin_path = _xfbin_path;
    xfbin.load(xfbin_path);
}

void XFBIN_Unpacker::unpack() {
    get_game();

    create_main_directory();

    write_index_json();

    create_page_directories();

    create_index_file();

    logger.send(Logger::Level::INFO, "Successfully unpacked XFBIN to directory {} ({}ms).", 
        logger.file(unpacked_directory_path.filename().string()), logger.end_timer(0));
}

void XFBIN_Unpacker::get_game() {
    std::string_view filename_fmt = logger.file(xfbin.filename);
    if (config.game == nucc::Game::UNKNOWN) {
        logger.info("Unpacking {} from no particular game...", filename_fmt);
    } else {
        logger.info("Unpacking {} from {}...", filename_fmt, nucc::game_to_string(config.game));
    }
    xfbin.game = config.game;
}

void XFBIN_Unpacker::create_main_directory() {
    unpacked_directory_path = xfbin.filename;
    if (!std::filesystem::create_directory(unpacked_directory_path)) {
        logger.error("Failed to create directory {}",
            unpacked_directory_path.string()
        );
        return;
    }
}

void XFBIN_Unpacker::write_index_json() {
    logger.send(Logger::Level::VERBOSE, "Writing {}...", logger.file("_index.json"));
    logger.start_timer(1);
    index_json["External_Name"] = xfbin.filename;
    index_json["Version"] = xfbin.version();
    index_json["Game"] = nucc::game_to_string(xfbin.game);
    for (auto& type : xfbin.types()) {
        index_json["Types"].push_back(type);
    }
    for (auto& path : xfbin.paths()) {
        index_json["Paths"].push_back(path);
    }
    for (auto& name : xfbin.names()) {
        index_json["Names"].push_back(name);
    }
    logger.send(Logger::Level::VERBOSE, "Writing complete! ({}ms)", logger.end_timer(1));
}

void XFBIN_Unpacker::create_page_directories() {
    size_t page_count = 0;
    for (const nucc::Page& page_data : xfbin.pages()) {
        XFBIN_Unpacker::Page page(this, page_data, page_count);
        process_page(page);
        page_count++;
    }
}

void XFBIN_Unpacker::process_page(Page page) {
    page.write_global_json();

    page.create_directory();

    page.process_chunks();

    page.write_file();
}

void XFBIN_Unpacker::Page::write_global_json() {
    json["Version"] = data.version();
    json["Chunk_Map_Offset"] = data.map_offset();
    json["Extra_Map_Offset"] = data.extra_offset();
}

void XFBIN_Unpacker::Page::create_directory() {
    // Use first non-null chunk as title.
    // !! Will change later to prioritise `nuccChunkClump`.
    for (const nucc::Chunk& chunk : data.chunks()) {
        if (chunk.type() == nucc::ChunkType::Null) continue;
        
        name = std::format("{:03} - {} ({})", index, chunk.name(), chunk.type_string());
        path = xfbin_unpacker->unpacked_directory_path / name;
        std::filesystem::create_directory(path);
        break;
    }
}

void XFBIN_Unpacker::Page::process_chunks() {
    size_t chunk_count = 0;
    for (const nucc::Chunk& chunk_data : data.chunks()) {
        Chunk chunk(chunk_data, chunk_count);
        process_chunk(chunk);
        chunk_count++;
    }
}

void XFBIN_Unpacker::Page::write_file() {
    std::ofstream file(path / "_page.json");
    file << json.dump(config.json_spacing);
}

void XFBIN_Unpacker::Page::process_chunk(Chunk chunk) {
    // Write each chunk's data to `_page.json`.
    auto& page_json_chunk = json["Chunks"][chunk.index];
    page_json_chunk["Type"] = chunk.data.type_string();
    if (chunk.data.path() != "")
        page_json_chunk["Path"] = chunk.data.path();
    if (chunk.data.name() != "")
        page_json_chunk["Name"] = chunk.data.name();
    page_json_chunk["Version"] = chunk.data.version();

    // Update `_index.json` to have total page count.
    xfbin_unpacker->index_json["Pages"][index][chunk.index] = json["Chunks"][chunk.index];

    // Create a file for each chunk.
    std::string filename_fmt = std::format("{:03} {} - {}{}", chunk.index, chunk.data.type_string().substr(9), "{}", "{}");
    if (chunk.data.type() == nucc::ChunkType::Null) {
        std::ofstream output(path / std::format("{:03} Null", chunk.index));
        output.close();
        return;
    }
    logger.send(Logger::Level::VERBOSE, "Converting chunk {}...", logger.file(chunk.data.name()));
    logger.start_timer(1);
    if (chunk.data.type() == nucc::ChunkType::Binary) {
        nucc::Binary buffer{&chunk};
        auto Parse_Binary = [&]<typename T>(T* t) {
            T binary_data{buffer.data()};
            std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
            output << binary_data.write_to_json().dump(config.json_spacing);
            output.close();
        };

        auto Dump_Binary = [&]() {
            buffer.dump().dump_file((
                path / std::vformat(filename_fmt, std::make_format_args(chunk.data.name(), ".binary"))
            ).string());
        };

        switch (config.game) {
            case nucc::Game::ASBR:
                if (chunk.name() == "messageInfo") {
                    nucc::ASBR::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(6, 3)};
                    std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
                    output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                    output.close();
                } else if (chunk.name() == "PlayerColorParam") {
                    Parse_Binary((nucc::ASBR::PlayerColorParam*)nullptr);
                } else if (chunk.name() == "SpeakingLineParam") {
                    Parse_Binary((nucc::ASBR::SpeakingLineParam*)nullptr);
                } else if (chunk.name() == "MainModeParam") {
                    Parse_Binary((nucc::ASBR::MainModeParam*)nullptr);
                } else {
                    Dump_Binary();
                }
                break;
            case nucc::Game::EOHPS4:
                if (chunk.name() == "messageInfo") {
                    nucc::EOHPS4::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(4, 3)};
                    std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
                    output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                    output.close();
                } else {
                    Dump_Binary();
                }
                break;
            case nucc::Game::EOHPS3:
                if (chunk.name() == "messageInfo_common" || chunk.name() == "messageInfo_adv" || chunk.name() == "messageInfo_btl") {
                    nucc::EOHPS3::messageInfo messageInfo{buffer.data(), (size_t)-1};
                    std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
                    output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                    output.close();
                } else {
                    Dump_Binary();
                }
                break;
            case nucc::Game::ASB:
                if (chunk.name() == "messageInfo") {
                    nucc::ASB::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(5, 3)};
                    std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
                    output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                    output.close();
                } else {
                    Dump_Binary();
                }
                break;
            default:    
                Dump_Binary();
        }
    } else {
        chunk.dump().dump_file((page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".bin"))).string());
    }
    logger.send(Logger::Level::VERBOSE, "Chunk converted! ({}ms)", logger.end_timer(1));
}

void XFBIN_Unpacker::create_index_file() {
    std::ofstream index_file(unpacked_directory_path / "_index.json");
    index_file << index_json.dump(config.json_spacing);
}
