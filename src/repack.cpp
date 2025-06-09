#include "main.hpp"

void Repack_XFBIN(std::filesystem::path& xfbin_path) {
    nucc::XFBIN xfbin;
    xfbin.name = xfbin_path.filename().string();

    if (!std::filesystem::exists(xfbin_path / "_index.json")) {
        logger.send(Logger::Level::FATAL, "{} could not be found in directory {}.", logger.file("_index.json"), logger.file(xfbin.name));
        logger.send(Logger::Level::INFO, "Ensure the directory was unpacked with this very tool. Others are incompatible.");
        return;
    }
    std::ifstream index_file(xfbin_path / "_index.json");
    nlohmann::json index_json = nlohmann::json::parse(index_file);
    index_file.close();
    xfbin.version = index_json["Version"];
    xfbin.game = nucc::string_to_game(index_json["Game"]);
    if (xfbin.game != nucc::Game::UNKNOWN)
        logger.send(Logger::Level::INFO, "Game detected: {}.", nucc::game_to_string(xfbin.game));

    config.game = xfbin.game;
    config.load("settings.json");

    if (config.game == nucc::Game::UNKNOWN) {
        logger.send(Logger::Level::INFO, "Repacking {} for no particular game...", logger.file(xfbin.name));
    } else {
        logger.send(Logger::Level::INFO, "Repacking {} for {}...", logger.file(xfbin.name), nucc::game_to_string(xfbin.game));
    }

    for (const auto& directory : std::filesystem::directory_iterator(xfbin_path)) {
        if (!directory.is_directory()) continue;

        if (!std::filesystem::exists(directory.path() / "_page.json")) {
            logger.send(Logger::Level::FATAL, "{} could not be found in directory {}.", logger.file("_page.json"), logger.file(directory.path().filename().string()));
            return;
        }
        std::ifstream page_file(directory.path() / "_page.json");
        nlohmann::json page_json = nlohmann::json::parse(page_file);
        page_file.close();

        auto page = xfbin.create_page();

        for (const auto& file : std::filesystem::directory_iterator(directory)) {
            std::string filename = file.path().filename().string();
            if (filename == "_page.json") continue;

            if (std::regex_match(filename, std::regex(R"(\d{3} Null)"))) {
                logger.send(Logger::Level::VERBOSE, "File found with type nuccChunkNull.");
                page->create_chunk(nucc::Chunk_Type::Null);
                continue;
            }

            std::regex regex(R"(\d{3}\s(.+?)\s-\s(.+))");
            std::smatch matches;
            std::regex_search(filename, matches, regex);
            auto chunk = page->create_chunk(nucc::string_to_chunk_type("nuccChunk" + matches[1].str()), "", logger.file(matches[2].str()));
            logger.send(Logger::Level::VERBOSE, "File {} found with type {}.", chunk->name, chunk->type_as_string());

            nucc::ASBR::messageInfo messageInfo;
            std::ifstream messageInfo_file(file.path());
            nlohmann::ordered_json messageInfo_json = nlohmann::ordered_json::parse(messageInfo_file);
            messageInfo.load(messageInfo_json);
            nucc::Binary binary_chunk;
            binary_chunk.load_data(messageInfo);
            binary_chunk.name = "messageInfo";
            binary_chunk.path = "WIN64/eng/230/messageInfo.bin";
            chunk->load(&binary_chunk);
        }
    }

    xfbin.write("test_output.xfbin");
}