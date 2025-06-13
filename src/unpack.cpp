#include "main.hpp"

XFBIN_Unpacker::XFBIN_Unpacker(const std::filesystem::path& _xfbin_path) {
    xfbin_path = _xfbin_path;
    xfbin.load(xfbin_path);
}

void XFBIN_Unpacker::unpack() {
    if (config.game == nucc::Game::UNKNOWN) {
        logger.info("Unpacking {} from no particular game...",
            logger.file(xfbin_path.filename().string())
        );
    } else {
        logger.info("Unpacking {} from {}...",
            logger.file(xfbin_path.filename().string()),
            nucc::game_to_string(config.game)
        );
    }
    xfbin.game = config.game;

    create_directory();
    write_index_json();

    // For each XFBIN page, create a separate directory.
    size_t page_count = 0;
    for (auto& page : xfbin.pages) {
        std::filesystem::path page_path;
        std::filesystem::path page_directory;

        // Write `_page.json` information.
        nlohmann::ordered_json page_json;
        page_json["Version"] = page.version;
        page_json["Unknown"] = page.unk;
        page_json["Chunk_Map_Offset"] = page.content.map_offset;
        page_json["Extra_Map_Offset"] = page.content.extra_offset;

        // Create page directory, using first non-null chunk as title.
        // (Will change later to prioritise `nuccChunkClump`.)
        for (auto& chunk : page.chunks) {
            if (chunk.type != nucc::Chunk_Type::Null) {
                page_path = std::format("{:03} - {} ({})", page_count, chunk.name, chunk.type_as_string());
                page_directory = unpacked_directory_path / page_path;
                std::filesystem::create_directory(page_directory);
                break;
            }
        }

        size_t chunk_index = 0;
        for (auto& chunk : page.chunks) {
            // Write each chunk's data to `_page.json`.
            auto& page_json_chunk = page_json["Chunks"][chunk_index];
            page_json_chunk["Type"] = chunk.type_as_string();
            if (chunk.path != "")
                page_json_chunk["Path"] = chunk.path;
            if (chunk.name != "")
                page_json_chunk["Name"] = chunk.name;
            page_json_chunk["Version"] = chunk.version;
            page_json_chunk["Unknown"] = chunk.unk;

            // Update `_index.json` to have total page count.
            index_json["Pages"][page_count][chunk_index] = page_json["Chunks"][chunk_index];

            // Create a file for each chunk.
            std::string filename_fmt = std::format("{:03} {} - {}{}", chunk_index, chunk.type_as_string().substr(9), "{}", "{}");
            if (chunk.type == nucc::Chunk_Type::Null) {
                std::ofstream output(page_directory / std::format("{:03} Null", chunk_index));
                output.close();
                chunk_index++;
                continue;
            }
            logger.send(Logger::Level::VERBOSE, "Converting chunk {}...", logger.file(chunk.name));
            logger.start_timer(1);
            if (chunk.type == nucc::Chunk_Type::Binary) {
                nucc::Binary buffer{&chunk};
                auto Parse_Binary = [&]<typename T>(T* t) {
                    T binary_data{buffer.data()};
                    std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".json")));
                    output << binary_data.write_to_json().dump(config.json_spacing);
                    output.close();
                };

                auto Dump_Binary = [&]() {
                    buffer.dump().dump_file((
                        page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".binary"))
                    ).string());
                };

                switch (config.game) {
                    case nucc::Game::ASBR:
                        if (chunk.name == "messageInfo") {
                            nucc::ASBR::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(6, 3)};
                            std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".json")));
                            output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                            output.close();
                        } else if (chunk.name == "PlayerColorParam") {
                            Parse_Binary((nucc::ASBR::PlayerColorParam*)nullptr);
                        } else if (chunk.name == "SpeakingLineParam") {
                            Parse_Binary((nucc::ASBR::SpeakingLineParam*)nullptr);
                        } else if (chunk.name == "MainModeParam") {
                            Parse_Binary((nucc::ASBR::MainModeParam*)nullptr);
                        } else {
                            Dump_Binary();
                        }
                        break;
                    case nucc::Game::EOHPS4:
                        if (chunk.name == "messageInfo") {
                            nucc::EOHPS4::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(4, 3)};
                            std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".json")));
                            output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                            output.close();
                        } else {
                            Dump_Binary();
                        }
                        break;
                    case nucc::Game::EOHPS3:
                        if (chunk.name == "messageInfo_common" || chunk.name == "messageInfo_adv" || chunk.name == "messageInfo_btl") {
                            nucc::EOHPS3::messageInfo messageInfo{buffer.data(), (size_t)-1};
                            std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".json")));
                            output << messageInfo.write_to_json("data/messageInfo_hashlist.bin").dump(config.json_spacing);
                            output.close();
                        } else {
                            Dump_Binary();
                        }
                        break;
                    case nucc::Game::ASB:
                        if (chunk.name == "messageInfo") {
                            nucc::ASB::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(5, 3)};
                            std::ofstream output(page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".json")));
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
                chunk.dump().dump_file((page_directory / std::vformat(filename_fmt, std::make_format_args(chunk.name, ".bin"))).string());
            }
            chunk_index++;
            logger.send(Logger::Level::VERBOSE, "Chunk converted! ({}ms)", logger.end_timer(1));
        }
        
        std::ofstream page_file(page_directory / "_page.json");
        page_file << page_json.dump(config.json_spacing);
        page_file.close();
        page_count++;
    }

    std::ofstream index_file(unpacked_directory_path / "_index.json");
    index_file << index_json.dump(config.json_spacing);
    index_file.close();

    logger.send(Logger::Level::INFO, "Successfully unpacked XFBIN to directory {} ({}ms).", 
        logger.file(unpacked_directory_path.filename().string()), logger.end_timer(0));
}

void XFBIN_Unpacker::create_directory() {
    unpacked_directory_path = xfbin.name;
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
    index_json["External_Name"] = xfbin.name;
    index_json["Version"] = xfbin.version;
    index_json["Game"] = nucc::game_to_string(xfbin.game);
    for (auto& type : xfbin.index.types) {
        index_json["Types"].push_back(type);
    }
    for (auto& path : xfbin.index.paths) {
        index_json["Paths"].push_back(path);
    }
    for (auto& name : xfbin.index.names) {
        index_json["Names"].push_back(name);
    }
    logger.send(Logger::Level::VERBOSE, "Writing complete! ({}ms)", logger.end_timer(1));
}