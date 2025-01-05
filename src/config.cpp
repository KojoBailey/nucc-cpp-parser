#include "main.hpp"

void Config::generate(std::filesystem::path path) {
    logger.send(Logger::Level::ERROR, "File `settings.json` not found. Generating new settings...");

    json["Previous_Game"] = "N/A";
    json["Default_Game"] = nucc::game_to_string(game);
    logger.send(Logger::Level::INFO, "Default Game set to {}.", json["Default_Game"].template get<std::string>());
    json["JSON_Spacing"] = json_spacing;
    logger.send(Logger::Level::INFO, "JSON Spacing set to {}.", json["JSON_Spacing"].template get<int>());

    update(path);
    logger.send(Logger::Level::INFO, "New settings generated. You can change these anytime in `settings.json`.");
}

void Config::load(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        generate(path);
    }
    std::ifstream file(path);
    json = nlohmann::ordered_json::parse(file);
    file.close();

    game = nucc::string_to_game(json["Default_Game"]);
    if (game == nucc::Game::UNKNOWN) {
        logger.send(Logger::Level::ERROR, "No recognisable game has been specified.");
        logger.send(Logger::Level::INFO, "Press 'Enter' to continue without a game, or enter the game's initials to set it.");
        logger.send(Logger::Level::INFO, "Send 'o' to see a full list of game options.");
        std::string input = str_lowercase(logger.get_input("Game: "));
        if (input == "o") {
            logger.send(Logger::Level::INFO, "Here are the currently supported game options:\n"
                "- ASBR   = JoJo's Bizarre Adventure: All-Star Battle R\n"
                "- EoHPS4 = JoJo's Bizarre Adventure: Eyes of Heaven (PS4)\n"
                "- EoHPS3 = JoJo's Bizarre Adventure: Eyes of Heaven (PS3)\n"
                "- ASB    = JoJo's Bizarre Adventure: All-Star Battle"
            );
            while (true) {
                logger.send(Logger::Level::INFO, "Press 'Enter' to continue without a game, or enter the game's initials to set it.");
                input = str_lowercase(logger.get_input("Game: "));
                if (input == "") {
                    break;
                } else if (nucc::string_to_game(input) == nucc::Game::UNKNOWN) {
                    logger.send(Logger::Level::ERROR, "\"{}\" isn't recognised as a game.", input);
                } else {
                    break;
                }
            }
        }
        game = nucc::string_to_game(input);
        json["Previous_Game"] = nucc::game_to_string(game);
        logger.send(Logger::Level::INFO, "Save this game as the default for future unpacking?");
        if (str_lowercase(logger.get_input("(y/n): ")) == "y") {
            json["Default_Game"] = json["Previous_Game"];
            logger.send(Logger::Level::INFO, "Default game set to {}.", json["Default_Game"].template get<std::string>());
        }
        update(path);
    }
    json_spacing = json["JSON_Spacing"];
}

void Config::update(std::filesystem::path path) {
    std::ofstream file(path);
    file << json.dump(json_spacing);
    file.close();
}