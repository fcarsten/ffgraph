#include "FFRuntimePaths.h"
#include "FFRuntimePathsConfig.h"
#include <cstdlib>
#include <filesystem>
#include <string>

namespace ffgraph_app {
void configureRuntimePaths(const char* argument) {
  namespace fs = std::filesystem;
  if (!std::getenv("FF_LIBDIR")) {
    std::error_code error;
    auto executable = fs::canonical("/proc/self/exe",error);
    if (error) executable = fs::absolute(argument);
    const auto build = fs::weakly_canonical(FFGRAPH_BUILD_BINARY_DIR);
    const auto resources = executable.parent_path() == build
        ? fs::path(FFGRAPH_BUILD_RESOURCE_DIR)
        : (executable.parent_path()/FFGRAPH_RESOURCE_RELATIVE).lexically_normal();
    setenv("FF_LIBDIR",resources.c_str(),0);
  }
  if (!std::getenv("FF_ICONDIR")) {
    const auto icons = fs::path(std::getenv("FF_LIBDIR"))/"icons";
    setenv("FF_ICONDIR",icons.c_str(),0);
  }
}
}
