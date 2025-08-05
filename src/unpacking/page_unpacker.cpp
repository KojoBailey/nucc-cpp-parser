#include "unpacker.hpp"

#include <cc2/asbr/binary/player_color_param.hpp>
#include <cc2/asbr/json/player_color_param.hpp>
#include <cc2/asbr/binary/message_info.hpp>
#include <cc2/asbr/json/message_info.hpp>

#include <nucc/utils/crc32.hpp>

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

void XFBIN_Unpacker::Page::Write_File() const {
    std::ofstream file(path / "_page.json");
    file << json.dump(config.json_spacing);
}

void XFBIN_Unpacker::Page::Handle_Chunk_Null(Chunk& chunk) const {
    std::ofstream output(path / std::format("{:03} Null", chunk.index));
}

void XFBIN_Unpacker::Page::Handle_Chunk_Binary(Chunk& chunk) {
    switch (config.game) {
        case nucc::game::asbr:
            if (Handle_ASBR(chunk)) return;
    }
    const std::string full_filename = chunk.filename + ".bin";
    chunk.data.storage()->dump_file((path / full_filename).string());
}

bool XFBIN_Unpacker::Page::Handle_ASBR(Chunk& chunk) {
    switch (nucc::crc32::hash(chunk.data.name())) {
        case nucc::crc32::hash("PlayerColorParam"):
            Parse_Data<cc2::asbr::player_color_param>(chunk);
            return true;
        case nucc::crc32::hash("messageInfo"):
            Parse_Data<cc2::asbr::message_info>(chunk);
            return true;
    }
    return false;
}

void XFBIN_Unpacker::Page::Process_Chunk(Chunk chunk) {
    chunk.Write_JSON();
    std::string chunk_index_str = std::format("{:03}", chunk.index);
    json["Chunks"][chunk_index_str] = chunk.json;
    xfbin_unpacker->index_json["Pages"][index][chunk_index_str] = chunk.json;

    // Create a file for each chunk.
    chunk.filename = std::format("{} - {}", chunk_index_str, chunk.data.name());
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