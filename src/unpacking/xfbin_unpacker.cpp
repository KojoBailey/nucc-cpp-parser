#include "unpacker.hpp"

XFBIN_Unpacker::XFBIN_Unpacker(const std::filesystem::path& _xfbin_path) {
    xfbin_path = _xfbin_path;
    xfbin.load(xfbin_path);
    log.show_debug = true;
    log.show_verbose = true;
}

void XFBIN_Unpacker::Unpack() {
    log.verbose("Getting game information...");
    Get_Game();

    log.verbose("Creating main directory...");
    Create_Main_Directory();

    log.verbose("Writing index JSON data...");
    // logger.start_timer(1);
    Write_Index_JSON();
    // logger.send(Logger::Level::VERBOSE, "Writing complete! ({}ms)", logger.end_timer(1));

    log.verbose("Creating page directories...");
    Create_Page_Directories();

    log.verbose("Writing \"_index.json\" file...");
    Create_Index_File();

    log.info(std::format("Successfully unpacked XFBIN to directory \"{}\".", unpacked_directory_path.filename().string()));
    // log.info(std::format("Successfully unpacked XFBIN to directory {} ({}ms).", 
    //     unpacked_directory_path.filename().string(), logger.end_timer(0));
}

void XFBIN_Unpacker::Get_Game() {
    if (config.game == nucc::game::unknown) {
        log.info(std::format("Unpacking \"{}\" from no particular game...", xfbin.filename));
    } else {
        log.info(std::format("Unpacking \"{}\" from {}...", xfbin.filename, nucc::game_to_string(config.game)));
    }
    xfbin.game = config.game;
}

void XFBIN_Unpacker::Create_Main_Directory() {
    unpacked_directory_path = xfbin.filename;
    std::filesystem::create_directory(unpacked_directory_path);
    if (!std::filesystem::exists(unpacked_directory_path)) {
        log.error(
            kojo::logger::status::null_file,
            std::format("Failed to create directory {}", unpacked_directory_path.string()),
            "Check that this parser has appropiate permissions to create files on your system."
        );
        return;
    }
}

void XFBIN_Unpacker::Write_Index_JSON() {
    index_json["Filename"] = xfbin.filename;
    index_json["Version"] = xfbin.version();
    index_json["Game"] = nucc::game_to_string(xfbin.game);
    for (auto& type : xfbin.types()) {
        index_json["Types"].push_back(type);
    }
    for (auto& path : xfbin.paths()) {
        index_json["Paths"].push_back(path);
    }
    for (auto& name : xfbin.names()) {
        index_json["Names"].push_back(name);
    }
}

void XFBIN_Unpacker::Create_Page_Directories() {
    size_t page_count = 0;
    for (const nucc::page& page_data : xfbin.pages()) {
        XFBIN_Unpacker::Page page(this, page_data, page_count);
        Process_Page(page);
        page_count++;
    }
}

void XFBIN_Unpacker::Process_Page(Page page) {

    page.Write_Global_JSON();

    page.Create_Directory();

    page.Process_Chunks();

    page.Write_File();
}

void XFBIN_Unpacker::Create_Index_File() {
    std::ofstream index_file(unpacked_directory_path / "_index.json");
    index_file << index_json.dump(config.json_spacing);
}
