#include <algorithm>
#include "utils/string_utils.h"

namespace string_utils {
  std::string toLowerCase(const std::string& input) {
      if (input.empty()) {
        return input;
      } 
      std::string result = input;
      std::transform(result.begin(), result.end(), result.begin(), ::tolower);
      return result;
  }

  std::string toUpperCase(const std::string& input) {
    if (input.empty()) {
      return input;
    }
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
  }
}