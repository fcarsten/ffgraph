#include "FFRuntimePaths.h"
#include "FFRuntimePathsConfig.h"
#include <ffgraph/FFPlatform.h>
#include <cstdlib>
#include <filesystem>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <vector>
#endif

namespace ffgraph_app {
void configureRuntimePaths(const char* argument) {
  namespace fs = std::filesystem;
  if (!std::getenv("FF_LIBDIR")) {
    std::error_code error;
#ifdef _WIN32
    // argv[0] may be relative or just a name from PATH. The Windows API also
    // works when the application is started by Explorer from another folder.
    std::vector<wchar_t> filename(512);
    DWORD length = 0;
    for (;;) {
      length = GetModuleFileNameW(nullptr, filename.data(), static_cast<DWORD>(filename.size()));
      if (!length) throw std::runtime_error("Cannot locate ffgraph executable");
      if (length < filename.size()) break;
      filename.resize(filename.size() * 2);
    }
    auto executable = fs::canonical(fs::path(std::wstring(filename.data(),length)),error);
#else
    auto executable = fs::canonical("/proc/self/exe",error);
#endif
    if (error) executable = fs::absolute(argument);
    // The configured build directory need not exist on an installed machine.
    // equivalent also handles Windows path aliases and case differences.
    const bool buildTree = fs::equivalent(executable.parent_path(), FFGRAPH_BUILD_BINARY_DIR, error);
    const auto resources = buildTree
        ? fs::path(FFGRAPH_BUILD_RESOURCE_DIR)
        : (executable.parent_path()/FFGRAPH_RESOURCE_RELATIVE).lexically_normal();
    ffgraph_platform::setEnvironment("FF_LIBDIR",resources.generic_string().c_str());
  }
  if (!std::getenv("FF_ICONDIR")) {
    const auto icons = fs::path(std::getenv("FF_LIBDIR"))/"icons";
    ffgraph_platform::setEnvironment("FF_ICONDIR",icons.generic_string().c_str());
  }
}
}
