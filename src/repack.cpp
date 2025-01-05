#include "main.hpp"

void Repack_XFBIN(std::filesystem::path& xfbin_path) {
    nucc::XFBIN xfbin;
    xfbin.name = xfbin_path.filename().string();

    std::ifstream index_file(xfbin_path / "_index.json");
    nlohmann::json index_json = nlohmann::json::parse(index_file);
    index_file.close();
    xfbin.version = index_json["Version"];
    xfbin.game = nucc::string_to_game(index_json["Game"]);

    if (xfbin.game != nucc::Game::UNKNOWN)
        logger.send(Logger::Level::INFO, "Game detected: {}.", nucc::game_to_string(xfbin.game));
    logger.send(Logger::Level::INFO, "Repacking XFBIN `{}`...", xfbin.name);

    // for (std::filesystem::directory_iterator directory : xfbin_path) {
        
    // }
}