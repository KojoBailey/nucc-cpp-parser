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

    if (xfbin.game == nucc::Game::UNKNOWN) {
        logger.send(Logger::Level::INFO, "Repacking {} for no particular game...", logger.file(xfbin.name));
    } else {
        logger.send(Logger::Level::INFO, "Game detected: {}.", nucc::game_to_string(xfbin.game));
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
    }
}