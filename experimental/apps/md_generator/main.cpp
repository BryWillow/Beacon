#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>

#include "message_generator.h"
#include "config_provider.h" // Ensure ConfigProvider is used

const std::string JSON_EXTENSION = ".json"; // Changed from constexpr std::string_view to std::string
const std::string ILLEGAL_CHARS = "<>:\"|?*"; // Changed from constexpr std::string_view to std::string

std::string toLower(const std::string& inString) {
    std::string lowerString = inString;
    std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower);
    return lowerString;
}

bool isPathWellFormed(const std::string& path) {
    try {
        // Check if the path can be constructed as a std::filesystem::path
        std::filesystem::path fsPath(path);
        if (fsPath.empty()) {
            return false;
        }

        // Check for illegal characters in the path
        for (char ch : ILLEGAL_CHARS) {
            if (path.find(ch) != std::string::npos) {
                std::cerr << "Path contains invalid character: " << ch << "\n";
                return false;
            }
        }

        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "The specified path is invalid: " << e.what() << "\n";
        return false;
    }
}

// Prints usage information for the program
void usage() {
    std::cerr << "Usage: md_generator <config.json> <output.dat>\n";
    std::cerr << "  <config.json> : Path to the input configuration file (must be a valid JSON file).\n";
    std::cerr << "  <output.dat>  : Path to the output file where generated messages will be saved.\n";
    std::cerr << "Notes:\n";
    std::cerr << "  - Paths must not contain invalid characters (e.g., < > : \" | ? *).\n";
    std::cerr << "  - Escape special characters (e.g., !, $, &) in the shell using quotes or backslashes.\n";
    std::cerr << "  - Ensure paths are valid for your operating system.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        usage();
        return 1;
    }

    // Validate the user didn't enter garbage by accident.
    std::string configPath = argv[1];
    if (!isPathWellFormed(configPath)) {
        std::cerr << "Invalid config file path: " << configPath << "\n";
        return 1;
    }

    // Confirm that the config file actually exists.
    if (!std::filesystem::exists(configPath)) {
        std::cerr << "Config file does not exist: " << configPath << "\n";
        return 1;
    }

    // Confirm that the user provided a json file.
    std::string fileExtension = std::filesystem::path(configPath).extension().string();
    if (toLower(fileExtension) != JSON_EXTENSION) {
        std::cerr << "Config file must have " << JSON_EXTENSION << " extension: " << configPath << "\n";
        return 1;
    }

    // Confirm that the output file could be created.
    std::string outputFilePath = argv[2];
    if (!isPathWellFormed(outputFilePath)) {
        std::cerr << "Invalid output file path: " << outputFilePath << "\n";
        return 1;
    }

    try {
        ConfigProvider configProvider(configPath);

        MessageGenerator generator(configPath);
        size_t numMessages = generator.getMessageCount();
        generator.generateMessages(outputFilePath, numMessages);
    }
    catch (const std::exception& e) {
        std::cerr << "[md_generator] Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}