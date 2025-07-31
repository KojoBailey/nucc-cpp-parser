#include "config.hpp"

#include <fstream>

std::string str_lowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return str;
}

void Config::generate(std::filesystem::path path) {
    log.info("File \"settings.json\" not found. Generating new settings...");

    json["Previous_Game"] = "N/A";
    json["Default_Game"] = nucc::game_to_string(game);
    log.info(std::format("Default Game set to {}.", json["Default_Game"].template get<std::string>()));
    json["JSON_Spacing"] = json_spacing;
    log.info(std::format("JSON Spacing set to {}.", json["JSON_Spacing"].template get<int>()));

    update(path);
    log.info("New settings generated. You can change these anytime in \"settings.json\".");
}

void Config::load(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        generate(path);
    }
    std::ifstream file(path);
    json = nlohmann::ordered_json::parse(file);
    file.close();

    if (game == nucc::game::unknown)
        game = nucc::string_to_game(json["Default_Game"]);
    if (game == nucc::game::unknown) {
        log.error(
            kojo::logger::status::bad_value,
            "No recognisable game has been specified.",
            "Ensure the game specified is supported by this parser."
        );
        while (true) {
            log.info("Press `Enter` to continue without a game, or enter the game's initials to set it.");
            log.info("Send \"opt\" to see a full list of game options.");
            std::string input = str_lowercase(log.get_input("Game: "));
            if (input == "opt") {
                log.info("Here are the currently supported game options:\n"
                    "- ASBR   = JoJo's Bizarre Adventure: All-Star Battle R\n"
                    "- EoHPS4 = JoJo's Bizarre Adventure: Eyes of Heaven (PS4)\n"
                    "- EoHPS3 = JoJo's Bizarre Adventure: Eyes of Heaven (PS3)\n"
                    "- ASB    = JoJo's Bizarre Adventure: All-Star Battle"
                );
            } else if (input == "") {
                break;
            } else if (nucc::string_to_game(input) == nucc::game::unknown) {
                log.error(
                    kojo::logger::status::bad_value,
                    std::format("\"{}\" isn't recognised as a game.", input),
                    "Enter a game that this parser supports, and check for typos."
                );
            } else {
                game = nucc::string_to_game(input);
                json["Previous_Game"] = nucc::game_to_string(game);
                log.info("Save this game as the default for future unpacking? (y/n)");
                if (str_lowercase(log.get_input(": ")) == "y") {
                    json["Default_Game"] = json["Previous_Game"];
                    log.info(std::format("Default game set to {}.", json["Default_Game"].template get<std::string>()));
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