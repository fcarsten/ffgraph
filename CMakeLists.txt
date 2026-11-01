cmake_minimum_required(VERSION 3.21)
project(ffgraph LANGUAGES C CXX VERSION 1.0)

include(GNUInstallDirs)
include(CTest)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TK REQUIRED IMPORTED_TARGET tk)
find_package(X11 REQUIRED)
find_library(XPM_LIBRARY NAMES Xpm REQUIRED)
find_path(XPM_INCLUDE_DIR NAMES X11/xpm.h REQUIRED)
find_library(MATH_LIBRARY NAMES m)
find_library(DL_LIBRARY NAMES dl)

option(FFGRAPH_SANITIZERS "Enable address and undefined-behavior sanitizers (GCC/Clang)" OFF)
if(FFGRAPH_SANITIZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "Sanitizer configuration requires GCC or Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# The modern GML parser/scanner is deliberately opt-in until a full
# token/diagnostic and graph-output parity audit is complete. It is an
# independent library, not linked into FFGraph or the application.
option(FFGRAPH_ENABLE_MODERN_GML "Build and test experimental GNU Bison/Flex GML parser" OFF)
if(FFGRAPH_ENABLE_MODERN_GML)
  add_subdirectory(tests/modern_gml)
endif()

# Build the checked-in legacy bison++/flex++ outputs verbatim.
# Grammar regeneration is deliberately NOT an implicit build dependency.
add_library(FFcontainer STATIC
  container/FFStacks.cc
  container/FFVHSets.cc
  container/FFVOHSets.cc
  container/StdContainer.cc
  container/FFLists.cc
  container/FFAVLSets.cc
  container/FFMPlex.cc
  container/FFAVLMaps.cc
  container/FFMFSets.cc
)
add_library(FFGraph STATIC
  fgraph/RomeParser.cc
  fgraph/RomeScanner.cc
  fgraph/GTParser.cc
  fgraph/GTScanner.cc
  fgraph/ffError.cc
  fgraph/FFParser.cc
  fgraph/FFScanner.cc
  fgraph/FFEdgeClass.cc
  fgraph/FFGraphClass.cc
  fgraph/FFNodeClass.cc
  fgraph/FFGraph.cc
  fgraph/FFLabelClass.cc
  fgraph/FFSubGraphLabel.cc
  fgraph/FFGraphLabel.cc
  fgraph/FFTINodeLabel.cc
  fgraph/FFTIEdgeLabel.cc
  fgraph/FFTIGraphLabel.cc
)
add_library(FFlayout STATIC
  flayout/FFShow.cc
  flayout/FFRandom.cc
  flayout/FFOrtho.cc
  flayout/FFOrthoInsp.cc
  flayout/FFNaiv.cc
  flayout/FFspringembedder.cc
  flayout/FFsugiyama.cc
  flayout/FFlayout_algorithmen.cc
  flayout/FFreduce_crossing.cc
  flayout/FFSpring.cc
  flayout/FFSugi.cc
  flayout/FFSpringInsp.cc
  flayout/FFSugiInsp.cc
  flayout/FFNaivInsp.cc
  flayout/FFimprove_position.cc
  flayout/FFKaese.cc
)
add_library(FFGraphic STATIC
  fgraphic/FFClusterWindow.cc
  fgraphic/FFTclBasicWindow.cc
  fgraphic/FFTclSubWindow.cc
  fgraphic/FFPoint.cc
  fgraphic/FFMatrix.cc
  fgraphic/FFWindow.cc
  fgraphic/FFGraphicNode.cc
  fgraphic/FFGraphicEdge.cc
  fgraphic/FFTclInspector.cc
  fgraphic/FFTclGraphInsp.cc
  fgraphic/FFTclWindow.cc
  fgraphic/FFTcl3DWindow.cc
  fgraphic/../misc/tkAppInit.c
  fgraphic/../misc/tkImgPmap.c
)

foreach(target FFcontainer FFGraph FFlayout FFGraphic)
  target_compile_features(${target} PUBLIC cxx_std_17)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS ON)
  target_include_directories(${target} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fgraph>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fgraphic>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/flayout>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/container>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/misc>
  )
  target_link_libraries(${target} PUBLIC PkgConfig::TK)
  target_compile_options(${target} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wno-unused-parameter>
  )
endforeach()
target_include_directories(FFGraphic PRIVATE ${XPM_INCLUDE_DIR})
target_link_libraries(FFGraphic PUBLIC FFGraph PkgConfig::TK X11::X11 ${XPM_LIBRARY})
add_executable(ffgraph ffmain.cc)
target_compile_features(ffgraph PRIVATE cxx_std_17)
# Preserve historical static archive resolution order.
target_link_libraries(ffgraph PRIVATE FFGraph FFGraphic FFlayout FFcontainer PkgConfig::TK X11::X11 ${XPM_LIBRARY})
if(MATH_LIBRARY)
  target_link_libraries(ffgraph PRIVATE ${MATH_LIBRARY})
endif()
if(DL_LIBRARY)
  target_link_libraries(ffgraph PRIVATE ${DL_LIBRARY})
endif()

install(TARGETS ffgraph RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink ffgraph \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/ffgraph3D\")")
install(DIRECTORY lib/ffgraph DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ffgraph/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ffgraph FILES_MATCHING PATTERN "*.h")
install(TARGETS FFGraph FFGraphic FFlayout FFcontainer ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(BUILD_TESTING)
  # Generator has no Bison/Tk dependency; exercise it in the regular suite.
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    add_test(NAME test_graph_corpus_generator_pass11
      COMMAND "${Python3_EXECUTABLE}" -m unittest discover
              -s "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus" -p test_generate.py)
    set_tests_properties(test_graph_corpus_generator_pass11 PROPERTIES TIMEOUT 30)
  endif()
  foreach(test_name
    test_dllist_stl
    test_hash_pointer_compat
    test_hashset_stl
    test_mplex_stl
    test_mplex_priority1
    test_container_mplex_pass8
    test_container_hash_pix_pass8
    test_container_avl_pass8
    test_container_avlmap_pass8
    test_container_avl_exception_followup
    test_container_dllist_pass8
    test_string_compat
    test_strstream_compat
    test_vhbag_stl
    test_vhmap_stl
  )
    add_executable(${test_name} tests/${test_name}.cpp)
    target_compile_features(${test_name} PRIVATE cxx_std_17)
    target_include_directories(${test_name} PRIVATE include .)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
  # FFGraph handles use intrusive reference counts. Exercise assignment and
  # destruction against the real model, not a mock of FFGraphClass.
  # Pass 9: graph incidence/value-copy invariants and headless routing/layout
  # regression tests. No GUI window or external graph fixture required.
  foreach(test_name
      test_graph_invariants_pass9
      test_graph_geometry_pass9
      test_graph_layout_pass9)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_compile_features(${test_name} PRIVATE cxx_std_17)
    target_link_libraries(${test_name} PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(${test_name} PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(${test_name} PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)
  endforeach()

  # Pass 10: end-to-end sgraph/GML load-modify-save-reload coverage,
  # including the bundled historical graph. This target needs no display.
  # Pass 11: optional GML graph-output differential test. Production keeps
  # GTControl; the Bison/Flex path must match real legacy graph semantics
  # before it can be considered for a runtime importer switch.
  if(FFGRAPH_ENABLE_MODERN_GML)
    add_executable(test_gml_graph_parity_pass11
      tests/modern_gml/test_gml_graph_parity_pass11.cpp
      fgraph/modern/gml_graph_adapter.cc)
    target_compile_features(test_gml_graph_parity_pass11 PRIVATE cxx_std_17)
    target_include_directories(test_gml_graph_parity_pass11 PRIVATE fgraph/modern)
    target_link_libraries(test_gml_graph_parity_pass11 PRIVATE
      ffgraph_modern_gml
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY})
    if(MATH_LIBRARY)
      target_link_libraries(test_gml_graph_parity_pass11 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_gml_graph_parity_pass11 PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_gml_graph_parity_pass11 COMMAND test_gml_graph_parity_pass11)
    set_tests_properties(test_gml_graph_parity_pass11 PROPERTIES TIMEOUT 45)
    add_dependencies(test_gml_graph_parity_pass11 ffgraph_gml_corpus_smoke)
    add_test(NAME test_gml_generated_graph_parity_pass11
      COMMAND test_gml_graph_parity_pass11
              "${CMAKE_CURRENT_BINARY_DIR}/tests/modern_gml/corpus-smoke")
    set_tests_properties(test_gml_generated_graph_parity_pass11 PROPERTIES TIMEOUT 120)
  endif()

  # Opt-in end-to-end graph importer and bounded spring-layout regression
  # against exactly the same deterministic parser/graph corpus.
  option(FFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS
    "Run generated-graph import/invariant and headless spring-layout tests" OFF)
  if(FFGRAPH_ENABLE_CORPUS_LAYOUT_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(FFGRAPH_LAYOUT_CORPUS "${CMAKE_CURRENT_BINARY_DIR}/generated-layout-corpus")
    add_custom_command(
      OUTPUT "${FFGRAPH_LAYOUT_CORPUS}/manifest.json"
      COMMAND "${Python3_EXECUTABLE}"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/generate.py"
              --output-dir "${FFGRAPH_LAYOUT_CORPUS}" --profile smoke --force
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/generate.py"
      VERBATIM)
    add_custom_target(ffgraph_layout_corpus
      DEPENDS "${FFGRAPH_LAYOUT_CORPUS}/manifest.json")
    add_executable(test_graph_corpus_layout_pass11
      tests/test_graph_corpus_layout_pass11.cpp)
    target_compile_features(test_graph_corpus_layout_pass11 PRIVATE cxx_std_17)
    target_link_libraries(test_graph_corpus_layout_pass11 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY})
    if(MATH_LIBRARY)
      target_link_libraries(test_graph_corpus_layout_pass11 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_graph_corpus_layout_pass11 PRIVATE ${DL_LIBRARY})
    endif()
    add_dependencies(test_graph_corpus_layout_pass11 ffgraph_layout_corpus)
    add_test(NAME test_graph_corpus_layout_pass11
      COMMAND test_graph_corpus_layout_pass11 "${FFGRAPH_LAYOUT_CORPUS}")
    set_tests_properties(test_graph_corpus_layout_pass11 PROPERTIES TIMEOUT 120)
  endif()

  add_executable(test_graph_io_pass10 tests/test_graph_io_pass10.cpp)
  target_compile_features(test_graph_io_pass10 PRIVATE cxx_std_17)
  target_compile_definitions(test_graph_io_pass10 PRIVATE
    FFGRAPH_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(test_graph_io_pass10 PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY}
  )
  if(MATH_LIBRARY)
    target_link_libraries(test_graph_io_pass10 PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_graph_io_pass10 PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_graph_io_pass10 COMMAND test_graph_io_pass10)
  set_tests_properties(test_graph_io_pass10 PROPERTIES TIMEOUT 40)

  add_executable(test_graph_ownership tests/test_graph_ownership.cpp)
  target_compile_features(test_graph_ownership PRIVATE cxx_std_17)
  target_link_libraries(test_graph_ownership PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY}
  )
  if(MATH_LIBRARY)
    target_link_libraries(test_graph_ownership PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_graph_ownership PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_graph_ownership COMMAND test_graph_ownership)

  # Graph labels/inspectors borrow their owning graph; exercise reference
  # cycles, the constructor guard, and copy-out ownership semantics.
  add_executable(test_graph_reference_cycle tests/test_graph_reference_cycle.cpp)
  target_link_libraries(test_graph_reference_cycle PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY}
  )
  if(MATH_LIBRARY)
    target_link_libraries(test_graph_reference_cycle PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_graph_reference_cycle PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_graph_reference_cycle COMMAND test_graph_reference_cycle)

  # Replacement labels must acquire their graph observer; interpreters
  # created before Tk initialization must be released on destruction.
  foreach(test_name test_graph_label_initialization test_ffwindow_interpreter_lifetime)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(${test_name} PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(${test_name} PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()

  # Ownership regression tests use the existing model and GUI archives.
  add_executable(test_label_replacement tests/test_label_replacement.cpp)
  target_link_libraries(test_label_replacement PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY}
  )
  if(MATH_LIBRARY)
    target_link_libraries(test_label_replacement PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_label_replacement PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_label_replacement COMMAND test_label_replacement)

  add_executable(test_tcl_command_lifetime tests/test_tcl_command_lifetime.cpp)
  target_link_libraries(test_tcl_command_lifetime PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY}
  )
  if(MATH_LIBRARY)
    target_link_libraries(test_tcl_command_lifetime PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_tcl_command_lifetime PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_tcl_command_lifetime COMMAND test_tcl_command_lifetime)

  add_executable(test_tcl_compat
      tests/test_tcl_compat.cpp
  )

  target_compile_features(test_tcl_compat PRIVATE cxx_std_17)

  target_include_directories(test_tcl_compat PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )

  target_link_libraries(test_tcl_compat PRIVATE
    PkgConfig::TK
  )

  add_test(NAME test_tcl_compat COMMAND test_tcl_compat)

  # Requires a display; run under Xvfb to exercise inspector hide/show.
  # Do not require Xvfb for ordinary compilation or headless unit tests.
  find_program(XVFB_RUN_EXECUTABLE NAMES xvfb-run)
  if(XVFB_RUN_EXECUTABLE)
    # Exercises real Tk root-window and Tcl interpreter teardown without
    # relying on WSLg or the user's interactive desktop.
    add_executable(test_ffwindow_tk_teardown tests/test_ffwindow_tk_teardown.cpp)
    target_link_libraries(test_ffwindow_tk_teardown PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(test_ffwindow_tk_teardown PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_ffwindow_tk_teardown PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_ffwindow_tk_teardown
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:test_ffwindow_tk_teardown>")
    set_tests_properties(test_ffwindow_tk_teardown PROPERTIES TIMEOUT 30)

    # Repeated two-root lifecycle, borrowed subwindow commands and nested
    # Tcl callbacks deleting their owning window. Independent of WSLg.
    add_executable(test_tk_lifecycle_pass10 tests/test_tk_lifecycle_pass10.cpp)
    target_compile_features(test_tk_lifecycle_pass10 PRIVATE cxx_std_17)
    target_link_libraries(test_tk_lifecycle_pass10 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(test_tk_lifecycle_pass10 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_tk_lifecycle_pass10 PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_tk_lifecycle_pass10
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:test_tk_lifecycle_pass10>")
    set_tests_properties(test_tk_lifecycle_pass10 PROPERTIES TIMEOUT 45)

    add_executable(test_inspector_visibility tests/test_inspector_visibility.cpp)
    target_link_libraries(test_inspector_visibility PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(test_inspector_visibility PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_inspector_visibility PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_inspector_visibility
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:test_inspector_visibility>")
  endif()
endif()
