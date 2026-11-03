#include "FFRuntimePaths.h"
#include "FFRuntimePathsConfig.h"
#include "test_check.h"
#include <cstdlib>
#include <filesystem>
#include <string>

int main(int argc, char** argv) {
  std::filesystem::current_path("/tmp");
  unsetenv("FF_LIBDIR"); unsetenv("FF_ICONDIR");
  ffgraph_app::configureRuntimePaths(argc ? argv[0] : "test_runtime_paths");
  FF_CHECK(std::string(std::getenv("FF_LIBDIR")) == FFGRAPH_BUILD_RESOURCE_DIR);
  FF_CHECK(std::filesystem::is_regular_file(std::filesystem::path(std::getenv("FF_LIBDIR"))/"tcl/plainGraph.tcl"));
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == std::string(FFGRAPH_BUILD_RESOURCE_DIR)+"/icons");
  setenv("FF_LIBDIR","/tmp/custom resources",1); unsetenv("FF_ICONDIR");
  ffgraph_app::configureRuntimePaths(argv[0]);
  FF_CHECK(std::string(std::getenv("FF_LIBDIR")) == "/tmp/custom resources");
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == "/tmp/custom resources/icons");
  setenv("FF_ICONDIR","/tmp/independent icons",1);
  ffgraph_app::configureRuntimePaths(argv[0]);
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == "/tmp/independent icons");
  std::cout << "build-tree runtime resources resolve outside the checkout; environment overrides retained\n";
}
