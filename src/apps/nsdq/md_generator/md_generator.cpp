// ---------------------------------------------------------------------------
// File        : md_generator.cpp
// Description : Market Data Generator for NSDQ ITCH messages (HFT-style)
// ---------------------------------------------------------------------------

#include <filesystem>
#include <iostream>
#include "nsdq/md_itch_generator.h"

namespace fs = std::filesystem;

int main()
{
    try
    {
        // Determine project root and data directory
        fs::path projectRoot = fs::path(__FILE__).parent_path().parent_path().parent_path();
        fs::path dataDir = projectRoot / "data";
        if (!fs::exists(dataDir))
            fs::create_directories(dataDir);

        // Default capture file
        fs::path defaultFile = dataDir / "default.itch";

        if (!fs::exists(defaultFile)) {
            std::cout << "[Generator] Creating default capture file..." << std::endl;
            ItchFileGenerator::Generate(defaultFile.string(), 10000); // generate 10k messages
            std::cout << "[Generator] Done generating " << defaultFile << std::endl;
        } else {
            std::cout << "[Generator] default.itch already exists, skipping generation." << std::endl;
        }
    }
    catch (const std::exception &e) {
        std::cerr << "[Generator] Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}