# Initial native Windows coverage. This can also run on Linux to exercise the
# same suite with Xpm disabled. The existing full Linux suite remains the default.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
foreach(test_name IN ITEMS
    test_dllist_stl test_hash_pointer_compat test_hashset_stl test_mplex_stl
    test_string_compat)
  add_executable(${test_name} tests/${test_name}.cpp)
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_include_directories(${test_name} PRIVATE include .)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

foreach(test_name IN ITEMS
    test_graph_invariants_pass9 test_graph_geometry_pass9 test_graph_layout_pass9
    test_sugiyama_failure test_sugiyama_numeric test_layout_controls
    test_graph_io_pass10 test_platform)
  add_executable(${test_name} tests/${test_name}.cpp)
  target_compile_features(${test_name} PRIVATE cxx_std_17)
  target_link_libraries(${test_name} PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK ${FFGRAPH_DISPLAY_LIBRARIES} ${MATH_LIBRARY} ${DL_LIBRARY})
  add_test(NAME ${test_name} COMMAND ${test_name})
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 120)
endforeach()
target_compile_definitions(test_graph_io_pass10 PRIVATE FFGRAPH_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME test_sugiyama_scalar_failure COMMAND test_sugiyama_failure --scalar)
set_tests_properties(test_sugiyama_scalar_failure PROPERTIES TIMEOUT 180)

add_executable(test_runtime_paths tests/test_runtime_paths.cpp misc/FFRuntimePaths.cc)
target_compile_features(test_runtime_paths PRIVATE cxx_std_17)
target_include_directories(test_runtime_paths PRIVATE include misc "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_test(NAME test_runtime_paths COMMAND test_runtime_paths)

add_executable(test_tcl_compat tests/test_tcl_compat.cpp)
target_compile_features(test_tcl_compat PRIVATE cxx_std_17)
target_include_directories(test_tcl_compat PRIVATE include)
target_link_libraries(test_tcl_compat PRIVATE PkgConfig::TK)
add_test(NAME test_tcl_compat COMMAND test_tcl_compat)

# Native Tk tests run directly on Windows; Linux uses a private Xvfb display.
find_program(XVFB_RUN_EXECUTABLE NAMES xvfb-run)
if(WIN32 OR XVFB_RUN_EXECUTABLE)
  set(initial_gui_command ${CMAKE_CROSSCOMPILING_EMULATOR})
  if(NOT WIN32)
    set(initial_gui_command "${XVFB_RUN_EXECUTABLE}" -a)
  endif()
  foreach(test_name IN ITEMS test_native_gui_smoke test_layout_controls_gui)
    add_executable(${test_name} tests/${test_name}.cpp misc/FFRuntimePaths.cc)
    target_compile_features(${test_name} PRIVATE cxx_std_17)
    target_include_directories(${test_name} PRIVATE misc "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_link_libraries(${test_name} PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK ${FFGRAPH_DISPLAY_LIBRARIES} ${MATH_LIBRARY} ${DL_LIBRARY})
    add_test(NAME ${test_name} COMMAND ${initial_gui_command} "$<TARGET_FILE:${test_name}>")
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 90 LABELS gui
      ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")
  endforeach()
  set(initial_runtime_display)
  if(NOT WIN32)
    set(initial_runtime_display "${XVFB_RUN_EXECUTABLE}" -a)
  endif()
  set(initial_runtime_emulator)
  foreach(emulator_argument IN LISTS CMAKE_CROSSCOMPILING_EMULATOR)
    list(APPEND initial_runtime_emulator "--emulator=${emulator_argument}")
  endforeach()
  add_test(NAME test_installed_runtime COMMAND ${initial_runtime_display}
    "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_installed_runtime.py"
    --build "${CMAKE_CURRENT_BINARY_DIR}" --libdir "${CMAKE_INSTALL_LIBDIR}" ${initial_runtime_emulator})
  set_tests_properties(test_installed_runtime PROPERTIES LABELS gui TIMEOUT 120)
endif()
