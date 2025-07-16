#include <nucc/game.hpp>

#include <nlohmann/json.hpp>
#include <filesystem>

class Config {
public:
    nlohmann::ordered_json json;
    
    nucc::Game game{nucc::Game::UNKNOWN};
    int json_spacing{2};

    void generate(std::filesystem::path path);
    void load(std::filesystem::path path);
private:
    void update(std::filesystem::path path);
};
inline Config config;