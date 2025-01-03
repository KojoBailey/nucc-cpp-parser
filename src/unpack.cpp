#include "main.hpp"

void Unpack_XFBIN(std::filesystem::path& xfbin_path) {
    // Load XFBIN into class.
    nucc::XFBIN xfbin{xfbin_path};

    // Create directory for XFBIN unpacked contents.
    std::filesystem::path main_directory = xfbin.name;
    std::filesystem::create_directory(main_directory);
    
    // Write `_index.json` information.
    nlohmann::ordered_json index_json;
    index_json["External Name"] = xfbin.name;
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

    // For each XFBIN page, create a separate directory.
    size_t page_count = 0;
    for (auto& page : xfbin.pages) {
        std::filesystem::path page_path;
        std::filesystem::path page_directory;

        // Write `_page.json` information.
        nlohmann::ordered_json page_json;
        page_json["Version"] = page.version;
        page_json["Unknown"] = page.unk;
        page_json["Chunk Map Offset"] = page.content.map_offset;
        page_json["Extra Map Offset"] = page.content.extra_offset;

        // Create page directory, using first non-null chunk as title.
        // (Will change later to prioritise `nuccChunkClump`.)
        for (auto& chunk : page.chunks) {
            if (chunk.type != nucc::Chunk_Type::Null) {
                page_path = std::format("{:03} - {} ({})", page_count, chunk.name, chunk.type_as_string());
                page_directory = main_directory / page_path;
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
            if (chunk.type == nucc::Chunk_Type::Null) {
                std::ofstream output(page_directory / std::format("{:03} - null", chunk_index));
                output.close();
            } else if (chunk.type == nucc::Chunk_Type::Binary) {
                nucc::Binary buffer{&chunk};
                auto Parse_Binary = [&]<typename T>(T* t) {
                    T binary_data{buffer.data()};
                    std::ofstream output(page_directory / std::format("{:03} - {}.json", chunk_index, chunk.name));
                    output << binary_data.write_to_json().dump(json_spacing);
                    output.close();
                };

                if (game == nucc::Game::ASBR) {
                    if (chunk.name == "messageInfo") {
                        nucc::ASBR::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(6, 3)};
                        std::ofstream output(page_directory / std::format("{:03} - {}.json", chunk_index, chunk.name));
                        output << messageInfo.write_to_json("./messageInfo_hashlist.bin").dump(json_spacing);
                        output.close();
                    } else if (chunk.name == "PlayerColorParam") {
                        Parse_Binary((nucc::ASBR::PlayerColorParam*)nullptr);
                    } else if (chunk.name == "SpeakingLineParam") {
                        Parse_Binary((nucc::ASBR::SpeakingLineParam*)nullptr);
                    } else if (chunk.name == "MainModeParam") {
                        Parse_Binary((nucc::ASBR::MainModeParam*)nullptr);
                    } else {
                        buffer.dump().dump_file((page_directory / std::format("{:03} - {}.binary", chunk_index, chunk.name)).string());
                    }
                } else if (game == nucc::Game::EOH) {
                    if (chunk.name == "messageInfo") {
                        nucc::EOH::messageInfo messageInfo{buffer.data(), (size_t)-1, chunk.path.substr(4, 3)};
                        std::ofstream output(page_directory / std::format("{:03} - {}.json", chunk_index, chunk.name));
                        output << messageInfo.write_to_json("./messageInfo_hashlist.bin").dump(json_spacing);
                        output.close();
                    } else {
                        buffer.dump().dump_file((page_directory / std::format("{:03} - {}.binary", chunk_index, chunk.name)).string());
                    }
                } else {
                    buffer.dump().dump_file((page_directory / std::format("{:03} - {}.binary", chunk_index, chunk.name)).string());
                }
            } else {
                chunk.dump().dump_file((page_directory / std::format("{:03} - {}.binary", chunk_index, chunk.name)).string());
            }
            chunk_index++;
        }
        
        std::ofstream page_file(page_directory / "_page.json");
        page_file << page_json.dump(json_spacing);
        page_file.close();
        page_count++;
    }

    std::ofstream index_file(main_directory / "_index.json");
    index_file << index_json.dump(json_spacing);
    index_file.close();
}