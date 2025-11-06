#include "string_utils.h"
#include <algorithm>
#include <cctype>

std::string normalizeString(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}
