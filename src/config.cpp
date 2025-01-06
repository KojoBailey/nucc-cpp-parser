#include "main.hpp"

void Config::generate(std::filesystem::path path) {
    logger.send(Logger::Level::ERROR, "File {} not found. Generating new settings...", logger.file("settings.json"));

    json["Previous_Game"] = "N/A";
    json["Default_Game"] = nucc::game_to_string(game);
    logger.send(Logger::Level::INFO, "Default Game set to {}.", json["Default_Game"].template get<std::string>());
    json["JSON_Spacing"] = json_spacing;
    logger.send(Logger::Level::INFO, "JSON Spacing set to {}.", json["JSON_Spacing"].template get<int>());

    update(path);
    logger.send(Logger::Level::INFO, "New settings generated. You can change these anytime in {}.", logger.file("settings.json"));
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
        while (true) {
            logger.send(Logger::Level::INFO, "Press {} to continue without a game, or enter the game's initials to set it.", logger.input("Enter"));
            logger.send(Logger::Level::INFO, "Send {} to see a full list of game options.", logger.input("o"));
            std::string input = str_lowercase(logger.get_input("Game: "));
            if (input == "o") {
                logger.send(Logger::Level::INFO, "Here are the currently supported game options:\n"
                    "- {}   = JoJo's Bizarre Adventure: All-Star Battle R\n"
                    "- {} = JoJo's Bizarre Adventure: Eyes of Heaven (PS4)\n"
                    "- {} = JoJo's Bizarre Adventure: Eyes of Heaven (PS3)\n"
                    "- {}    = JoJo's Bizarre Adventure: All-Star Battle",
                logger.input("ASBR"), logger.input("EoHPS4"), logger.input("EoHPS3"), logger.input("ASB"));
            } else if (input == "") {
                break;
            } else if (nucc::string_to_game(input) == nucc::Game::UNKNOWN) {
                logger.send(Logger::Level::ERROR, "\"{}\" isn't recognised as a game.", input);
            } else {
                game = nucc::string_to_game(input);
                json["Previous_Game"] = nucc::game_to_string(game);
                logger.send(Logger::Level::INFO, "Save this game as the default for future unpacking? ({}/{})", logger.input("y"), logger.input("n"));
                if (str_lowercase(logger.get_input(": ")) == "y") {
                    json["Default_Game"] = json["Previous_Game"];
                    logger.send(Logger::Level::INFO, "Default game set to {}.", json["Default_Game"].template get<std::string>());
                }
                break;
            }
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