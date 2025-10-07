// ---------------------------------------------------------------------------
// @file        itch_file_generator.h
// Description : Stub for NSDQ ITCH file generator
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once
#include <string>
#include <cstddef>
#include <iostream>

namespace beacon::nsdq::market_data::itch
{

    // ---------------------------------------------------------------------------
    // This stub exists to allow the md_generator app to build independently.
    // It does NOT generate real ITCH files. When the real generator is available,
    // it will live in the versioned directory (e.g., v5), but the include path
    // remains the same so client code does not need to change.
    // ---------------------------------------------------------------------------
    struct ItchFileGenerator
    {
        static void Generate(const std::string &filename, std::size_t numMessages)
        {
            // Stub implementation: just prints an info message
            (void)numMessages;
            std::cout << "[Stub] Would generate " << numMessages
                      << " ITCH messages to " << filename << std::endl;
        }
    };

} // namespace beacon::nsdq::market_data::itch