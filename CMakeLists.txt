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
  foreach(test_name
    test_dllist_stl
    test_hash_pointer_compat
    test_hashset_stl
    test_mplex_stl
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
endif()
