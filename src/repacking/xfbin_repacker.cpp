#include "repacker.hpp"

XFBIN_Repacker::XFBIN_Repacker(const std::filesystem::path& _xfbin_path) {
    xfbin_path = _xfbin_path;
    xfbin.filename = xfbin_path.filename().string();
}

void XFBIN_Repacker::Repack() {
    Read_Index_JSON();

    Get_Game();

    Read_Pages();

    Write_XFBIN();
}

void XFBIN_Repacker::Read_Index_JSON() {
    std::filesystem::path index_path = xfbin_path / "_index.json";
    if (!std::filesystem::exists(index_path)) {
        log.fatal(
            kojo::logger::status::null_file,
            std::format("Could not find index data in directory \"{}\".", xfbin.filename),
            "Ensure the directory contains an \"index.json\" file and that it was unpacked with this same tool."
        );
        return;
    }

    std::ifstream index_file(index_path);
    nlohmann::json index_json = nlohmann::json::parse(index_file);
    index_file.close();
}

void XFBIN_Repacker::Get_Game() {
    xfbin.game = nucc::string_to_game(index_json["Game"]);

    if (xfbin.game != nucc::game::unknown) {
        log.info(std::format("Game detected: {}",  xfbin.game));
        config.game = xfbin.game;
    } else {
        config.load("settings.json");
        xfbin.game = config.game;
        if (config.game == nucc::game::unknown) {
            log.info(std::format("Repacking \"{}\" for no particular game...", xfbin.filename));
            return;
        }
    }
    log.info(std::format("Repacking \"{}\" for {}", xfbin.filename, nucc::game_to_string(xfbin.game)));
}

void XFBIN_Repacker::Read_Pages() {
    for (auto& directory : std::filesystem::directory_iterator(xfbin_path) {
        if (!directory.is_directory()) continue;

        Page page{directory.path()};
        page.Repack();
    }
}

void repack_xfbin(const std::filesystem::path& xfbin_path) {
    for (const auto& directory : std::filesystem::directory_iterator(xfbin_path)) {
        auto page = xfbin.create_page();

        for (const auto& file : std::filesystem::directory_iterator(directory)) {
            std::string filename = file.path().filename().string();
            if (filename == "_page.json") continue;

            if (std::regex_match(filename, std::regex(R"(\d{3} Null)"))) {
                logger.send(Logger::Level::VERBOSE, "File found with type nuccChunkNull.");
                page->create_chunk(nucc::ChunkType::Null);
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