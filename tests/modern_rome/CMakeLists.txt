cmake_minimum_required(VERSION 3.21)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(ffgraph_modern_rome LANGUAGES CXX)
  include(CTest)
  option(FFGRAPH_MODERN_ROME_SANITIZERS "Instrument standalone Rome parser" OFF)
  if(FFGRAPH_MODERN_ROME_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
  endif()
endif()

find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)
set(ROME_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../fgraph/modern")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
BISON_TARGET(ModernRomeParser "${ROME_SOURCE_DIR}/modern_rome.y"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_rome_parser.cpp"
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_rome_parser.hpp"
  COMPILE_FLAGS "--report=state -Wall -Werror")
FLEX_TARGET(ModernRomeScanner "${ROME_SOURCE_DIR}/modern_rome.l"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/modern_rome_scanner.cpp")
ADD_FLEX_BISON_DEPENDENCY(ModernRomeScanner ModernRomeParser)
add_library(ffgraph_modern_rome STATIC ${BISON_ModernRomeParser_OUTPUTS}
  ${FLEX_ModernRomeScanner_OUTPUTS} "${ROME_SOURCE_DIR}/rome_records.cc")
target_compile_features(ffgraph_modern_rome PUBLIC cxx_std_17)
target_include_directories(ffgraph_modern_rome PUBLIC "${ROME_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_options(ffgraph_modern_rome PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
  find_package(Threads REQUIRED)
  add_executable(test_rome_modern_pass12 test_rome_modern_pass12.cpp)
  target_link_libraries(test_rome_modern_pass12 PRIVATE ffgraph_modern_rome Threads::Threads)
  add_test(NAME test_rome_modern_pass12 COMMAND test_rome_modern_pass12)
  set_tests_properties(test_rome_modern_pass12 PROPERTIES LABELS headless TIMEOUT 120)
endif()

if(TARGET FFGraph)
  add_library(ffgraph_modern_rome_graph STATIC "${ROME_SOURCE_DIR}/rome_graph_adapter.cc")
  target_link_libraries(ffgraph_modern_rome_graph PUBLIC ffgraph_modern_rome FFGraph)
  if(BUILD_TESTING)
    add_executable(test_rome_compatibility_pass12 test_rome_compatibility_pass12.cpp)
    target_compile_definitions(test_rome_compatibility_pass12 PRIVATE
      FFGRAPH_TEST_SOURCE_DIR="${CMAKE_CURRENT_LIST_DIR}/../..")
    target_link_libraries(test_rome_compatibility_pass12 PRIVATE ffgraph_modern_rome_graph
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY} ${MATH_LIBRARY} ${DL_LIBRARY})
    add_test(NAME test_rome_compatibility_pass12 COMMAND test_rome_compatibility_pass12)
    add_test(NAME test_rome_generated_oracle_pass12 COMMAND test_rome_compatibility_pass12 --generated)
    add_test(NAME test_rome_modern_failures_pass12 COMMAND test_rome_compatibility_pass12 --failures)
    set_tests_properties(test_rome_compatibility_pass12 test_rome_generated_oracle_pass12
      test_rome_modern_failures_pass12 PROPERTIES LABELS headless TIMEOUT 120)
  endif()
endif()
