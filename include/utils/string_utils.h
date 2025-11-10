#pragma once

#include <string>

namespace string_utils {
    /// @brief Normalize a string by converting it to lowercase.
    /// @param input The input string.
    /// @return The normalized string.
    std::string toLowerCase(const std::string& input);

    /// @brief Normalize a string by converting it to uppercase.
    /// @param input The input string.
    /// @return The normalized string.
    std::string toUpperCase(const std::string& input);
}