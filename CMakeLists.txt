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
option(FFGRAPH_GUI_STRESS_TESTS "Enable slower, large-graph Tk/Xvfb workflow tests" OFF)
option(FFGRAPH_ENABLE_MODERN_ROME_PILOT
  "Build isolated GNU Bison/Flex Rome pilot and compatibility tests" OFF)
# Reject a stale selection rather than silently replacing the requested backend.
if(DEFINED FFGRAPH_FF_BACKEND AND NOT FFGRAPH_FF_BACKEND STREQUAL "modern")
  message(FATAL_ERROR
    "Legacy FF has been retired. Remove the obsolete backend selection: "
    "cmake -U FFGRAPH_FF_BACKEND -U FFGRAPH_ENABLE_MODERN_FF_PILOT ...")
endif()
if(FFGRAPH_SANITIZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "Sanitizer configuration requires GCC or Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# FF and GML use generated GNU Bison/Flex parsers in all CMake builds.
# Rome remains historical.
# Remove obsolete cache entries from older build directories if necessary:
#   cmake -S . -B build/... -U FFGRAPH_GML_BACKEND -U FFGRAPH_ENABLE_MODERN_GML
if(DEFINED FFGRAPH_GML_BACKEND AND FFGRAPH_GML_BACKEND STREQUAL "legacy")
  message(FATAL_ERROR
    "Legacy GML has been retired. Reconfigure without FFGRAPH_GML_BACKEND=legacy "
    "(cmake -U FFGRAPH_GML_BACKEND -U FFGRAPH_ENABLE_MODERN_GML ...).")
endif()
add_subdirectory(tests/modern_gml)

# Rome's legacy parser remains checked-in and is not regenerated implicitly.
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
  fgraph/ffError.cc
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
# Share the modern graph adapter between FFGraphClass::load() and tests.
# Parser/scanner output remains generated in the build directory.
target_sources(FFGraph PRIVATE fgraph/modern/gml_graph_adapter.cc)
target_link_libraries(FFGraph PUBLIC ffgraph_modern_gml)

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
# Installed FFGraph static archives now depend on this generated parser.
install(TARGETS ffgraph_modern_gml ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory(tests/modern_ff)
target_sources(FFGraph PRIVATE fgraph/modern/ff_graph_adapter.cc)
target_link_libraries(FFGraph PUBLIC ffgraph_modern_ff)
install(TARGETS ffgraph_modern_ff ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(BUILD_TESTING)
  add_executable(test_check_ndebug tests/test_check_ndebug.cpp)
  target_compile_features(test_check_ndebug PRIVATE cxx_std_17)
  target_compile_definitions(test_check_ndebug PRIVATE NDEBUG)
  add_test(NAME test_check_ndebug
    COMMAND "${CMAKE_COMMAND}" "-DTEST_BINARY=$<TARGET_FILE:test_check_ndebug>"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_ndebug.cmake")

  # Generator has no Bison/Tk dependency; exercise it in the regular suite.
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    add_test(NAME test_graph_corpus_generator_pass11
      COMMAND "${Python3_EXECUTABLE}" -m unittest discover
              -s "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus" -p "test_*.py")
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

  # Exercise the public FFGraphClass::load() entry point independently of
  # direct event replay.
  add_executable(test_gml_production_backend_pass11
    tests/modern_gml/test_gml_production_backend_pass11.cpp)
  target_compile_features(test_gml_production_backend_pass11 PRIVATE cxx_std_17)
  target_link_libraries(test_gml_production_backend_pass11 PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY})
  if(MATH_LIBRARY)
    target_link_libraries(test_gml_production_backend_pass11 PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_gml_production_backend_pass11 PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_gml_production_backend_pass11
    COMMAND test_gml_production_backend_pass11)
  set_tests_properties(test_gml_production_backend_pass11 PROPERTIES TIMEOUT 45)

  # Pass 10: end-to-end sgraph/GML load-modify-save-reload coverage,
  # including the bundled historical graph. This target needs no display.
  # Golden regression cases plus generated graph-oracle checks.
  # Legacy GTControl was retired; reference expectations are data, not code.
  add_executable(test_gml_graph_regression_pass11
    tests/modern_gml/test_gml_graph_regression_pass11.cpp)
  target_compile_features(test_gml_graph_regression_pass11 PRIVATE cxx_std_17)
  target_include_directories(test_gml_graph_regression_pass11 PRIVATE fgraph/modern)
  target_link_libraries(test_gml_graph_regression_pass11 PRIVATE
    ffgraph_modern_gml
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY})
  if(MATH_LIBRARY)
    target_link_libraries(test_gml_graph_regression_pass11 PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_gml_graph_regression_pass11 PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_gml_graph_regression_pass11 COMMAND test_gml_graph_regression_pass11)
  set_tests_properties(test_gml_graph_regression_pass11 PROPERTIES TIMEOUT 45)
  add_dependencies(test_gml_graph_regression_pass11 ffgraph_gml_corpus_smoke)
  add_test(NAME test_gml_generated_graph_regression_pass11
    COMMAND test_gml_graph_regression_pass11
            "${CMAKE_CURRENT_BINARY_DIR}/tests/modern_gml/corpus-smoke")
  set_tests_properties(test_gml_generated_graph_regression_pass11 PROPERTIES TIMEOUT 120)

  # Corpus: compare production loads and direct event replay, and check
  # generated graphs against independent oracle data (not legacy code).
  # A separate profile controls run-time cost.
  set(FFGRAPH_GML_DIFFERENTIAL_PROFILE "smoke" CACHE STRING
    "Generated GML differential corpus profile (smoke, standard, stress)")
  set_property(CACHE FFGRAPH_GML_DIFFERENTIAL_PROFILE PROPERTY STRINGS
    smoke standard stress)
  if(NOT FFGRAPH_GML_DIFFERENTIAL_PROFILE MATCHES "^(smoke|standard|stress)$")
    message(FATAL_ERROR "Unsupported FFGRAPH_GML_DIFFERENTIAL_PROFILE")
  endif()
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  set(FFGRAPH_DIFF_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus")
  set(FFGRAPH_DIFF_GRAPHS
    "${CMAKE_CURRENT_BINARY_DIR}/gml-differential-${FFGRAPH_GML_DIFFERENTIAL_PROFILE}")
  set(FFGRAPH_DIFF_META
    "${CMAKE_CURRENT_BINARY_DIR}/gml-differential-metamorphic-${FFGRAPH_GML_DIFFERENTIAL_PROFILE}")
  file(GLOB FFGRAPH_DIFF_SAMPLES CONFIGURE_DEPENDS
    "${FFGRAPH_DIFF_SOURCE}/samples/*.gml")
  add_custom_command(
    OUTPUT "${FFGRAPH_DIFF_GRAPHS}/manifest.json"
    COMMAND "${Python3_EXECUTABLE}" "${FFGRAPH_DIFF_SOURCE}/generate.py"
      --output-dir "${FFGRAPH_DIFF_GRAPHS}"
      --profile "${FFGRAPH_GML_DIFFERENTIAL_PROFILE}"
      --examples-dir "${FFGRAPH_DIFF_SOURCE}/samples" --force
    DEPENDS "${FFGRAPH_DIFF_SOURCE}/generate.py" ${FFGRAPH_DIFF_SAMPLES}
    COMMENT "Generate complete GML differential graph/syntax corpus"
    VERBATIM)
  add_custom_command(
    OUTPUT "${FFGRAPH_DIFF_META}/manifest.json"
    COMMAND "${Python3_EXECUTABLE}" "${FFGRAPH_DIFF_SOURCE}/metamorphic.py"
      --output-dir "${FFGRAPH_DIFF_META}"
      --profile "${FFGRAPH_GML_DIFFERENTIAL_PROFILE}" --force
    DEPENDS "${FFGRAPH_DIFF_SOURCE}/generate.py" "${FFGRAPH_DIFF_SOURCE}/metamorphic.py"
    COMMENT "Generate paired metamorphic differential GML corpus"
    VERBATIM)
  add_custom_target(ffgraph_gml_differential_fixtures DEPENDS
    "${FFGRAPH_DIFF_GRAPHS}/manifest.json"
    "${FFGRAPH_DIFF_META}/manifest.json")
  add_executable(test_graph_snapshot_pass11
    tests/graph_corpus/test_graph_snapshot_pass11.cpp)
  target_compile_features(test_graph_snapshot_pass11 PRIVATE cxx_std_17)
  target_include_directories(test_graph_snapshot_pass11 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME test_graph_snapshot_pass11 COMMAND test_graph_snapshot_pass11)
  set_tests_properties(test_graph_snapshot_pass11 PROPERTIES TIMEOUT 15)
  add_executable(test_gml_corpus_oracle_pass11
    tests/modern_gml/test_gml_corpus_oracle_pass11.cpp)
  target_compile_features(test_gml_corpus_oracle_pass11 PRIVATE cxx_std_17)
  target_include_directories(test_gml_corpus_oracle_pass11 PRIVATE fgraph/modern)
  target_link_libraries(test_gml_corpus_oracle_pass11 PRIVATE
    ffgraph_modern_gml
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY})
  if(MATH_LIBRARY)
    target_link_libraries(test_gml_corpus_oracle_pass11 PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_gml_corpus_oracle_pass11 PRIVATE ${DL_LIBRARY})
  endif()
  # Semantic validation must not mutate an existing graph on failure.
  # Builds with real graph classes, independently of Bison/Flex event input.
  add_executable(test_gml_semantic_atomicity_pass11
    tests/modern_gml/test_gml_semantic_atomicity_pass11.cpp)
  target_compile_features(test_gml_semantic_atomicity_pass11 PRIVATE cxx_std_17)
  target_include_directories(test_gml_semantic_atomicity_pass11 PRIVATE fgraph/modern)
  target_link_libraries(test_gml_semantic_atomicity_pass11 PRIVATE
    ffgraph_modern_gml
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY})
  if(MATH_LIBRARY)
    target_link_libraries(test_gml_semantic_atomicity_pass11 PRIVATE ${MATH_LIBRARY})
  endif()
  if(DL_LIBRARY)
    target_link_libraries(test_gml_semantic_atomicity_pass11 PRIVATE ${DL_LIBRARY})
  endif()
  add_test(NAME test_gml_semantic_atomicity_pass11
    COMMAND test_gml_semantic_atomicity_pass11)
  set_tests_properties(test_gml_semantic_atomicity_pass11 PROPERTIES TIMEOUT 45)
  add_dependencies(test_gml_corpus_oracle_pass11 ffgraph_gml_differential_fixtures)
  add_test(NAME test_gml_corpus_oracle_pass11
    COMMAND test_gml_corpus_oracle_pass11 graphs "${FFGRAPH_DIFF_GRAPHS}" 2)
  add_test(NAME test_gml_metamorphic_oracle_pass11
    COMMAND test_gml_corpus_oracle_pass11 metamorphic "${FFGRAPH_DIFF_META}" 2)
  if(FFGRAPH_GML_DIFFERENTIAL_PROFILE STREQUAL "stress")
    set(FFGRAPH_DIFF_TIMEOUT 900)
  else()
    set(FFGRAPH_DIFF_TIMEOUT 300)
  endif()
  set_tests_properties(test_gml_corpus_oracle_pass11
    test_gml_metamorphic_oracle_pass11
    PROPERTIES TIMEOUT ${FFGRAPH_DIFF_TIMEOUT})

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

    # Metamorphic input transformations verify parser/import invariants and
    # bounded layout robustness without relying on exact final XY positions.
    set(FFGRAPH_METAMORPHIC_CORPUS "${CMAKE_CURRENT_BINARY_DIR}/generated-metamorphic-corpus")
    add_custom_command(
      OUTPUT "${FFGRAPH_METAMORPHIC_CORPUS}/manifest.json"
      COMMAND "${Python3_EXECUTABLE}"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/metamorphic.py"
              --output-dir "${FFGRAPH_METAMORPHIC_CORPUS}" --profile smoke --force
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/metamorphic.py"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/generate.py"
      COMMENT "Regenerate GML metamorphic import/layout fixtures"
      VERBATIM)
    add_custom_target(ffgraph_metamorphic_corpus
      DEPENDS "${FFGRAPH_METAMORPHIC_CORPUS}/manifest.json")
    add_executable(test_graph_corpus_metamorphic_pass11
      tests/test_graph_corpus_metamorphic_pass11.cpp)
    target_compile_features(test_graph_corpus_metamorphic_pass11 PRIVATE cxx_std_17)
    target_link_libraries(test_graph_corpus_metamorphic_pass11 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY})
    if(MATH_LIBRARY)
      target_link_libraries(test_graph_corpus_metamorphic_pass11 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_graph_corpus_metamorphic_pass11 PRIVATE ${DL_LIBRARY})
    endif()
    add_dependencies(test_graph_corpus_metamorphic_pass11 ffgraph_metamorphic_corpus)
    add_test(NAME test_graph_corpus_metamorphic_pass11
      COMMAND test_graph_corpus_metamorphic_pass11 "${FFGRAPH_METAMORPHIC_CORPUS}")
    set_tests_properties(test_graph_corpus_metamorphic_pass11 PROPERTIES TIMEOUT 180)
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

  add_executable(test_ff_production_backend_pass12 tests/test_ff_production_backend_pass12.cpp)
  target_link_libraries(test_ff_production_backend_pass12 PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY} ${MATH_LIBRARY} ${DL_LIBRARY})
  add_test(NAME test_ff_production_backend_pass12 COMMAND test_ff_production_backend_pass12)
  set_tests_properties(test_ff_production_backend_pass12 PROPERTIES TIMEOUT 40)

  add_executable(test_rome_baseline_pass12 tests/test_rome_baseline_pass12.cpp)
  target_compile_definitions(test_rome_baseline_pass12 PRIVATE
    FFGRAPH_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(test_rome_baseline_pass12 PRIVATE
    "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
    PkgConfig::TK X11::X11 ${XPM_LIBRARY} ${MATH_LIBRARY} ${DL_LIBRARY})
  add_test(NAME test_rome_baseline_pass12 COMMAND test_rome_baseline_pass12)
  set_tests_properties(test_rome_baseline_pass12 PROPERTIES TIMEOUT 40)

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
    # File > Load must keep a newly loaded graph/window alive after its
    # temporary FFGraph handle goes out of scope. Test with a real Tk root.
    add_executable(test_load_window_lifetime_pass11
      tests/test_load_window_lifetime_pass11.cpp)
    target_link_libraries(test_load_window_lifetime_pass11 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(test_load_window_lifetime_pass11 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_load_window_lifetime_pass11 PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_load_window_lifetime_pass11
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a
              "$<TARGET_FILE:test_load_window_lifetime_pass11>"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase.gml")
    set_tests_properties(test_load_window_lifetime_pass11 PROPERTIES
      TIMEOUT 45
      ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")

    add_executable(test_ff_window_oracle_pass12 tests/test_ff_window_oracle_pass12.cpp)
    target_link_libraries(test_ff_window_oracle_pass12 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY} ${MATH_LIBRARY} ${DL_LIBRARY})
    add_test(NAME test_ff_window_oracle_pass12
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:test_ff_window_oracle_pass12>"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/modern_ff/fixtures/gui.g")
    set_tests_properties(test_ff_window_oracle_pass12 PROPERTIES TIMEOUT 60
      ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")

    # Drive the real "recall modPress l/m/w" Tcl commands with a test file
    # picker. Exercises multiwindow load, merge, cancel, semantic rejection,
    # repeated close/reopen, a larger historical file and nested callbacks.
    # Failure atomicity and window ownership are production guarantees.
    add_executable(test_gui_file_workflow_pass11
      tests/test_gui_file_workflow_pass11.cpp)
    target_link_libraries(test_gui_file_workflow_pass11 PRIVATE
      "$<LINK_GROUP:RESCAN,FFGraph,FFGraphic,FFlayout,FFcontainer>"
      PkgConfig::TK X11::X11 ${XPM_LIBRARY}
    )
    if(MATH_LIBRARY)
      target_link_libraries(test_gui_file_workflow_pass11 PRIVATE ${MATH_LIBRARY})
    endif()
    if(DL_LIBRARY)
      target_link_libraries(test_gui_file_workflow_pass11 PRIVATE ${DL_LIBRARY})
    endif()
    add_test(NAME test_gui_file_workflow_pass11
      COMMAND "${XVFB_RUN_EXECUTABLE}" -a
              "$<TARGET_FILE:test_gui_file_workflow_pass11>"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase.gml"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase3.gml"
              "${CMAKE_CURRENT_SOURCE_DIR}/tests/gui_fixtures/invalid-edge-target.gml")
    set_tests_properties(test_gui_file_workflow_pass11 PROPERTIES
      TIMEOUT 120
      ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")
    add_test(NAME test_ff_gui_file_workflow_pass12
        COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:test_gui_file_workflow_pass11>"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/modern_ff/fixtures/gui.g"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/modern_ff/fixtures/structure.g"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/gui_fixtures/truncated-ff.g")
    set_tests_properties(test_ff_gui_file_workflow_pass12 PROPERTIES TIMEOUT 120
        ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")
    if(FFGRAPH_GUI_STRESS_TESTS)
      add_test(NAME test_gui_large_file_workflow_pass11
        COMMAND "${XVFB_RUN_EXECUTABLE}" -a
                "$<TARGET_FILE:test_gui_file_workflow_pass11>"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase.gml"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase3.gml"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/gui_fixtures/invalid-edge-target.gml"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_corpus/samples/nase2.gml")
      set_tests_properties(test_gui_large_file_workflow_pass11 PROPERTIES
        TIMEOUT 300
        ENVIRONMENT "FF_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph;FF_ICONDIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/ffgraph/icons")
    endif()

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

if(FFGRAPH_ENABLE_MODERN_ROME_PILOT)
  # Deliberately not linked into FFGraph or the application yet.
  add_subdirectory(tests/modern_rome)
endif()

if(BUILD_TESTING)
  # Parser subdirectories label their own tests, also in standalone builds.
  get_property(headless_tests DIRECTORY PROPERTY TESTS)
  set_tests_properties(${headless_tests} PROPERTIES LABELS headless)
  # All tests requiring Xvfb belong to the GUI partition, including stress tests.
  foreach(gui_test IN ITEMS
      test_ffwindow_tk_teardown test_load_window_lifetime_pass11
      test_ff_window_oracle_pass12 test_gui_file_workflow_pass11
      test_ff_gui_file_workflow_pass12 test_gui_large_file_workflow_pass11
      test_tk_lifecycle_pass10 test_inspector_visibility)
    if(TEST ${gui_test})
      set_tests_properties(${gui_test} PROPERTIES LABELS gui)
    endif()
  endforeach()
endif()
