#include "repacker.hpp"

void XFBIN_Repacker::Page::Repack() {
    Read_JSON();
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