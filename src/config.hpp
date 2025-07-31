#pragma once

#include <nucc/game.hpp>

#include <kojo/logger.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>

class Config {
public:
    nlohmann::ordered_json json;
    
    nucc::game game{nucc::game::unknown};
    int json_spacing{2};

    void generate(std::filesystem::path path);
    void load(std::filesystem::path path);
private:
    kojo::logger log{"Config"};

    void update(std::filesystem::path path);
};
inline Config config;