#include "unpacker.hpp"

#include <nucc/utils/crc32.hpp>

XFBIN_Unpacker::XFBIN_Unpacker(const std::filesystem::path& _xfbin_path) {
    xfbin_path = _xfbin_path;
    xfbin.load(xfbin_path);
    log.show_debug = true;
    log.show_verbose = true;
}

void XFBIN_Unpacker::Unpack() {
    log.verbose("Getting game information...");
    Get_Game();

    log.verbose("Creating main directory...");
    Create_Main_Directory();

    log.verbose("Writing index JSON data...");
    // logger.start_timer(1);
    Write_Index_JSON();
    // logger.send(Logger::Level::VERBOSE, "Writing complete! ({}ms)", logger.end_timer(1));

    log.verbose("Creating page directories...");
    Create_Page_Directories();

    log.verbose("Writing \"_index.json\" file...");
    Create_Index_File();

    log.info(std::format("Successfully unpacked XFBIN to directory {}.", unpacked_directory_path.filename().string()));
    // log.info(std::format("Successfully unpacked XFBIN to directory {} ({}ms).", 
    //     unpacked_directory_path.filename().string(), logger.end_timer(0));
}

void XFBIN_Unpacker::Get_Game() {
    if (config.game == nucc::game::unknown) {
        log.info(std::format("Unpacking \"{}\" from no particular game...", xfbin.filename));
    } else {
        log.info(std::format("Unpacking \"{}\" from {}...", xfbin.filename, nucc::game_to_string(config.game)));
    }
    xfbin.game = config.game;
}

void XFBIN_Unpacker::Create_Main_Directory() {
    unpacked_directory_path = xfbin.filename;
    if (!std::filesystem::create_directory(unpacked_directory_path)) {
        log.error(
            kojo::logger::status::null_file,
            std::format("Failed to create directory {}", unpacked_directory_path.string()),
            "Check that this parser has appropiate permissions to create files on your system."
        );
        return;
    }
}

void XFBIN_Unpacker::Write_Index_JSON() {
    index_json["Filename"] = xfbin.filename;
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
}

void XFBIN_Unpacker::Create_Page_Directories() {
    size_t page_count = 0;
    for (const nucc::page& page_data : xfbin.pages()) {
        XFBIN_Unpacker::Page page(this, page_data, page_count);
        Process_Page(page);
        page_count++;
    }
}

void XFBIN_Unpacker::Process_Page(Page page) {
    
    page.Write_Global_JSON();

    page.Create_Directory();

    page.Process_Chunks();

    page.Write_File();
}

void XFBIN_Unpacker::Page::Write_Global_JSON() {
    json["Version"] = data.version();
    json["Chunk_Map_Offset"] = data.map_offset();
    json["Extra_Map_Offset"] = data.extra_offset();
}

void XFBIN_Unpacker::Page::Create_Directory() {
    // Use first non-null chunk as title.
    // !! Will change later to prioritise `nuccChunkClump`.
    for (const nucc::chunk& chunk : data.chunks()) {
        if (chunk.type() == nucc::chunk_type::null) continue;
        
        name = std::format("{:03} - {} ({})", index, chunk.name(), chunk.type_string());
        path = xfbin_unpacker->unpacked_directory_path / name;
        std::filesystem::create_directory(path);
        break;
    }
}

void XFBIN_Unpacker::Page::Process_Chunks() {
    size_t chunk_count = 0;
    for (const nucc::chunk& chunk_data : data.chunks()) {
        Chunk chunk(chunk_data, chunk_count);
        Process_Chunk(chunk);
        chunk_count++;
    }
}

void XFBIN_Unpacker::Page::Write_File() {
    std::ofstream file(path / "_page.json");
    file << json.dump(config.json_spacing);
}

void XFBIN_Unpacker::Page::Handle_Chunk_Null(Chunk& chunk) {
    std::ofstream output(path / std::format("{:03} Null", chunk.index));
}

void XFBIN_Unpacker::Page::Handle_Chunk_Binary(Chunk& chunk) {
    // switch (config.game) {
    //     case nucc::game::asbr:
    //         if (Handle_ASBR(chunk)) return;
    // }
    const std::string full_filename = chunk.filename + ".binary";
    chunk.data.storage()->dump_file((path / full_filename).string());
}

// bool XFBIN_Unpacker::Page::Handle_ASBR(Chunk& chunk) {
//     switch (nucc::crc32::hash(chunk.data.name())) {
//         case nucc::crc32::hash("PlayerColorParam"):
//             Parse_Data<nucc::asbr::player_color_param>(chunk);
//             return true;
//     }
//     return false;
// }

void XFBIN_Unpacker::Page::Process_Chunk(Chunk chunk) {
    chunk.Write_JSON();
    json["Chunks"][chunk.index] = chunk.json;
    xfbin_unpacker->index_json["Pages"][index][chunk.index] = chunk.json;

    // Create a file for each chunk.
    chunk.filename = std::format("{:03} {} - {}", chunk.index, chunk.data.type_string().substr(9), chunk.data.name());
    switch (chunk.data.type()) {
        case nucc::chunk_type::null:
            Handle_Chunk_Null(chunk);
            break;
        case nucc::chunk_type::binary:
            Handle_Chunk_Binary(chunk);
            break;
        default:
            const std::string full_filename = chunk.filename + ".bin";
            chunk.data.storage()->dump_file((path / full_filename).string());
    }

    /* DELETE */
    // if (chunk.data.type() == nucc::chunk_type::null) {
    //     std::ofstream output(path / std::format("{:03} Null", chunk.index));
    //     return;
    // }
    // logger.send(Logger::Level::VERBOSE, "Converting chunk {}...", logger.file(chunk.data.name()));
    // logger.start_timer(1);
    // if (chunk.data.type() == nucc::chunk_type::binary) {
        
    //     switch (config.game) {
    //         case nucc::game::asbr:
    //             if (chunk.name() == "messageInfo") {
    //                 nucc::asbr::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(6, 3)};
    //                 std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
    //                 output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
    //                 output.close();
    //             } else if (chunk.name() == "PlayerColorParam") {
    //                 Parse_Binary((nucc::asbr::player_color_param*)nullptr);
    //             } else if (chunk.name() == "SpeakingLineParam") {
    //                 Parse_Binary((nucc::asbr::SpeakingLineParam*)nullptr);
    //             } else if (chunk.name() == "MainModeParam") {
    //                 Parse_Binary((nucc::asbr::MainModeParam*)nullptr);
    //             } else {
    //                 Dump_Binary();
    //             }
    //             break;
    //         case nucc::game::EOHPS4:
    //             if (chunk.name() == "messageInfo") {
    //                 nucc::EOHPS4::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(4, 3)};
    //                 std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
    //                 output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
    //                 output.close();
    //             } else {
    //                 Dump_Binary();
    //             }
    //             break;
    //         case nucc::game::EOHPS3:
    //             if (chunk.name() == "messageInfo_common" || chunk.name() == "messageInfo_adv" || chunk.name() == "messageInfo_btl") {
    //                 nucc::EOHPS3::messageInfo messageInfo{buffer.data(), (size_t)-1};
    //                 std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
    //                 output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
    //                 output.close();
    //             } else {
    //                 Dump_Binary();
    //             }
    //             break;
    //         case nucc::game::ASB:
    //             if (chunk.name() == "messageInfo") {
    //                 nucc::ASB::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(5, 3)};
    //                 std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".json")));
    //                 output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
    //                 output.close();
    //             } else {
    //                 Dump_Binary();
    //             }
    //             break;
    //         default:    
    //             Dump_Binary();
    //     }
    // } else {
    //     chunk.dump().dump_file((page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name(), ".bin"))).string());
    // }
    // logger.send(Logger::Level::VERBOSE, "Chunk converted! ({}ms)", logger.end_timer(1));
}

void XFBIN_Unpacker::Page::Chunk::Write_JSON() {
    json["Type"] = data.type_string();
    if (data.path() != "")
        json["Path"] = data.path();
    if (data.name() != "")
        json["Name"] = data.name();
    json["Version"] = data.version();
}

void XFBIN_Unpacker::Create_Index_File() {
    std::ofstream index_file(unpacked_directory_path / "_index.json");
    index_file << index_json.dump(config.json_spacing);
}
