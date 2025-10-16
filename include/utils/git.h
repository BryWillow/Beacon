#include <iostream>
#include <string>
#include <cstdio>
#include <exception>

namespace beacon::utils::git {
  class GitUtils {
    public:
      static std::string getRepositoryRoot() {
      FILE* pipe = popen("git rev-parse --show-toplevel 2>/dev/null", "r");
      if (!pipe) {
        throw std::runtime_error("Unable to find repository root.");
      }
      char buffer[128];
      std::string result = "";
      while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
          result += buffer;
        }
      }
      pclose(pipe);
      if (!result.empty() && result.back() == '\n') {
        result.pop_back();
      }
      return result;
    }
  };
}