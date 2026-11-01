# Isolated Pass 12 pilot. This archive is never linked into ffgraph/FFGraph.
find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
BISON_TARGET(ModernFfParser
  "${PROJECT_SOURCE_DIR}/fgraph/modern/modern_ff.y"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_ff_parser.cpp"
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_ff_parser.hpp"
  COMPILE_FLAGS "--report=state")
FLEX_TARGET(ModernFfScanner
  "${PROJECT_SOURCE_DIR}/fgraph/modern/modern_ff.l"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_ff_scanner.cpp")
ADD_FLEX_BISON_DEPENDENCY(ModernFfScanner ModernFfParser)
add_library(ffgraph_modern_ff STATIC
  ${BISON_ModernFfParser_OUTPUTS} ${FLEX_ModernFfScanner_OUTPUTS}
  "${PROJECT_SOURCE_DIR}/fgraph/modern/ff_events.cc")
target_compile_features(ffgraph_modern_ff PUBLIC cxx_std_17)
target_include_directories(ffgraph_modern_ff PUBLIC
  "${PROJECT_SOURCE_DIR}/fgraph/modern"
  "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_options(ffgraph_modern_ff PRIVATE -Wall -Wextra)
if(BUILD_TESTING)
  add_executable(test_ff_compatibility_pass12
    test_ff_compatibility_pass12.cpp
    "${PROJECT_SOURCE_DIR}/fgraph/modern/ff_graph_adapter.cc")
  target_link_libraries(test_ff_compatibility_pass12 PRIVATE ffgraph_modern_ff
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY} ${MATH_LIBRARY} ${DL_LIBRARY})
  target_compile_definitions(test_ff_compatibility_pass12 PRIVATE
    FFGRAPH_TEST_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
  add_test(NAME test_ff_compatibility_pass12 COMMAND test_ff_compatibility_pass12)
  add_test(NAME test_ff_legacy_corpus_pass12 COMMAND test_ff_compatibility_pass12 --legacy-corpus)
  add_test(NAME test_ff_modern_failures_pass12 COMMAND test_ff_compatibility_pass12 --parser-failures)
  set_tests_properties(test_ff_compatibility_pass12 test_ff_legacy_corpus_pass12
    test_ff_modern_failures_pass12 PROPERTIES TIMEOUT 120)
endif()
