// ---------------------------------------------------------------------------
// @file        main.cpp
// Project     : Beacon
// Component   : NSDQ ITCH File Generator
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include "nsdq/market_data/itch/current/messages_active.h"
#include "file_generator.h"
#include "utils/git.h"

namespace beacongit = beacon::utils::git;

#include <iostream>
#include <string>
#include <cstdlib>

void printUsage() {
    std::cout << "Usage:\n"
                 "  file_generator [--help] [--messages <num>] [--file <path>]\n\n"
                 "Options:\n"
                 "  --help       Show this help message\n"
                 "  --messages   Number of messages to generate\n"
                 "  --file       Output file name\n\n"
                 "Purpose:\n"
                 "  Generates binary ITCH v5.0 messages in raw format.\n";
}

int main(int argc, char** argv) {
    size_t messageCount = 10000; // default
    std::string outputFile = "data/default.itch"; // default

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        else if (arg == "--messages" || arg == "-m") {
            if (i + 1 >= argc) {
                std::cerr << "ERROR: missing value for --messages.\n\n";
                printUsage();
                return 1;
            }
            std::string val = argv[++i];
            try {
                size_t idx = 0;
                messageCount = std::stoull(val, &idx);
                if (idx != val.size()) {
                    throw std::invalid_argument("partial parse");
                }
            } catch (...) {
                std::cerr << "ERROR: number of messages must be a valid integer.\n\n";
                printUsage();
                return 1;
            }
        }
        else if (arg == "--file" || arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "ERROR: missing value for --file.\n\n";
                printUsage();
                return 1;
            }
            outputFile = argv[++i];
        }
        else {
            std::cerr << "ERROR: unknown argument: " << arg << "\n\n";
            printUsage();
            return 1;
        }
    }

    try {
        ItchFileGenerator generator;
        std::string fullPath = beacongit::GitUtils::getRepositoryRoot() + outputFile;
        std::cout << "Writing generated file to: " << fullPath << "." << std::endl;

        generator.Generate(fullPath, messageCount);
        std::cout << "Generated " << messageCount
                  << " messages in " << fullPath << "." << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "ERROR: generating messages: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}