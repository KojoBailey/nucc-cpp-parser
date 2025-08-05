#include "repacker.hpp"

#include <cc2/asbr/binary/message_info.hpp>
#include <cc2/asbr/json/message_info.hpp>

void XFBIN_Repacker::Page::Repack() {
    Read_JSON();

    Create_Chunks();
}

void XFBIN_Repacker::Page::Read_JSON() {
    if (!std::filesystem::exists(path / "_page.json")) {
        log.fatal(
            kojo::logger::status::null_file,
            std::format("\"_page.json\" could not be found in directory {}.", path.filename().string()),
            "Check that prior unpacking was error-free, and that \"page.json\" was not deleted."
        );
        return;
    }
    std::ifstream page_file(path / "_page.json");
    json = nlohmann::json::parse(page_file);
    page_file.close();
}

void XFBIN_Repacker::Page::Create_Chunks() {
    nucc::page page;

    for (const auto& [index, entry] : json["Chunks"].items()) {
        std::string_view type_str = entry.at("Type").get<std::string_view>();
        nucc::chunk_type type = nucc::string_to_chunk_type(type_str);

        Chunk chunk;
        chunk.data.set_type(type);
        chunk.json = entry;
        chunk.index = index;
        chunk.directory = path;

        switch (type) {
            case nucc::chunk_type::binary:
                chunk.Handle_Chunk_Binary();
                break;
            default: break;
        }

        log.debug("Adding...");
        page.add_chunk(chunk.data);
        log.debug("Done.");
    }
}

void XFBIN_Repacker::Page::Chunk::Handle_Chunk_Binary() {
    data.set_name(json.at("Name").get<std::string_view>());
    data.set_path(json.at("Path").get<std::string_view>());
    data.set_version(json.at("Version").get<std::uint32_t>());

    filename = std::format("{} - {}", index, data.name());
    if (std::filesystem::exists(directory / (filename + ".json"))) {
        std::ifstream binary_file(directory / (filename + ".json"));
        nlohmann::ordered_json binary_json = nlohmann::ordered_json::parse(binary_file);
        auto binary_data = cc2::json_serializer<cc2::asbr::message_info>::read(binary_json);
        kojo::binary binary_ser = cc2::binary_serializer<cc2::asbr::message_info>::write(binary_data);
        data.update_data(binary_ser.data(), binary_ser.size());

        log.debug(std::format("Size: {}", data.meta<nucc::chunk_binary>()->size()));
    } else if (std::filesystem::exists(directory / (filename + ".bin"))) {
        log.debug("Found BIN file.");
    } else {
        log.error(
            kojo::logger::status::null_file,
            std::format(R"(Could not find file with name "{}".)", filename),
            R"(Ensure this file exists as documented in the "_page.json" and that it is JSON or BIN.)"
        );
    }
}
