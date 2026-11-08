#include "FFRuntimePaths.h"
#include "FFRuntimePathsConfig.h"
#include "test_check.h"
#include <ffgraph/FFPlatform.h>
#include <cstdlib>
#include <filesystem>
#include <string>

int main(int argc, char** argv) {
  std::filesystem::current_path(std::filesystem::temp_directory_path());
#ifdef _WIN32
  _putenv_s("FF_LIBDIR", ""); _putenv_s("FF_ICONDIR", "");
#else
  unsetenv("FF_LIBDIR"); unsetenv("FF_ICONDIR");
#endif
  ffgraph_app::configureRuntimePaths(argc ? argv[0] : "test_runtime_paths");
  FF_CHECK(std::string(std::getenv("FF_LIBDIR")) == FFGRAPH_BUILD_RESOURCE_DIR);
  FF_CHECK(std::filesystem::is_regular_file(std::filesystem::path(std::getenv("FF_LIBDIR"))/"tcl/plainGraph.tcl"));
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == std::string(FFGRAPH_BUILD_RESOURCE_DIR)+"/icons");
  ffgraph_platform::setEnvironment("FF_LIBDIR","/tmp/custom resources");
#ifdef _WIN32
  _putenv_s("FF_ICONDIR", "");
#else
  unsetenv("FF_ICONDIR");
#endif
  ffgraph_app::configureRuntimePaths(argv[0]);
  FF_CHECK(std::string(std::getenv("FF_LIBDIR")) == "/tmp/custom resources");
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == "/tmp/custom resources/icons");
  ffgraph_platform::setEnvironment("FF_ICONDIR","/tmp/independent icons");
  ffgraph_app::configureRuntimePaths(argv[0]);
  FF_CHECK(std::string(std::getenv("FF_ICONDIR")) == "/tmp/independent icons");
  std::cout << "build-tree runtime resources resolve outside the checkout; environment overrides retained\n";
}
