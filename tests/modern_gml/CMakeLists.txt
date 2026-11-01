cmake_minimum_required(VERSION 3.21)
project(ffgraph_modern_gml LANGUAGES CXX)
include(CTest)

# This option works when building tests/modern_gml independently, without
# requiring the full ffgraph project's sanitizer configuration.
option(FFGRAPH_MODERN_GML_SANITIZERS "Build the GML pilot with ASan/UBSan" OFF)
if(FFGRAPH_MODERN_GML_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# This standalone project intentionally needs neither Tcl/Tk nor libXpm.
# It can also be included by the top-level ffgraph build as an opt-in target.
find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

BISON_TARGET(ModernGmlParser
  "${CMAKE_CURRENT_LIST_DIR}/../../fgraph/modern/modern_gml.y"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_gml_parser.cpp"
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_gml_parser.hpp"
)
FLEX_TARGET(ModernGmlScanner
  "${CMAKE_CURRENT_LIST_DIR}/../../fgraph/modern/modern_gml.l"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_gml_scanner.cpp"
)
ADD_FLEX_BISON_DEPENDENCY(ModernGmlScanner ModernGmlParser)

add_library(ffgraph_modern_gml STATIC
  ${BISON_ModernGmlParser_OUTPUTS}
  ${FLEX_ModernGmlScanner_OUTPUTS}
  "${CMAKE_CURRENT_LIST_DIR}/../../fgraph/modern/gml_events.cc"
)
target_compile_features(ffgraph_modern_gml PUBLIC cxx_std_17)
target_include_directories(ffgraph_modern_gml PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/../../fgraph/modern"
  "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_compile_options(ffgraph_modern_gml PRIVATE -Wall -Wextra)

if(BUILD_TESTING)
  add_executable(test_gml_modern_pass11 "${CMAKE_CURRENT_LIST_DIR}/test_gml_modern_pass11.cpp")
  target_link_libraries(test_gml_modern_pass11 PRIVATE ffgraph_modern_gml)
  add_test(NAME test_gml_modern_pass11 COMMAND test_gml_modern_pass11)
  set_tests_properties(test_gml_modern_pass11 PROPERTIES TIMEOUT 30)
endif()
