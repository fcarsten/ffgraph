# Stage native DLLs and the Tcl/Tk script libraries with the application. The
# resulting installation can run without MSYS2's bin directory on PATH.
pkg_get_variable(FFGRAPH_TCL_PREFIX tcl prefix)
pkg_get_variable(FFGRAPH_TK_PREFIX tk prefix)
set(ffgraph_runtime_directories "${FFGRAPH_TCL_PREFIX}/bin" "${FFGRAPH_TK_PREFIX}/bin")
foreach(runtime_dll IN ITEMS libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
  execute_process(COMMAND "${CMAKE_CXX_COMPILER}" "-print-file-name=${runtime_dll}"
    OUTPUT_VARIABLE runtime_path OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
  if(IS_ABSOLUTE "${runtime_path}")
    get_filename_component(runtime_directory "${runtime_path}" DIRECTORY)
    list(APPEND ffgraph_runtime_directories "${runtime_directory}")
  endif()
endforeach()
set(FFGRAPH_WINDOWS_RUNTIME_SEARCH_DIRS "" CACHE STRING
  "Additional native Windows DLL search directories for installation")
list(APPEND ffgraph_runtime_directories ${FFGRAPH_WINDOWS_RUNTIME_SEARCH_DIRS})
list(REMOVE_DUPLICATES ffgraph_runtime_directories)

# Set the PE inspection tool explicitly, including for Linux-hosted MinGW
# builds. Cross-installs additionally need Windows system DLLs available in a
# search directory; native installations resolve those from Windows itself.
install(CODE "set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM windows+pe)
set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL objdump)
set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND \"${CMAKE_OBJDUMP}\")")
install(RUNTIME_DEPENDENCY_SET ffgraph_runtime
  DIRECTORIES ${ffgraph_runtime_directories}
  PRE_EXCLUDE_REGEXES "^[Aa][Pp][Ii]-[Mm][Ss]-" "^[Ee][Xx][Tt]-[Mm][Ss]-"
  POST_EXCLUDE_REGEXES ".*[Ss][Yy][Ss][Tt][Ee][Mm]32/.*"
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY "${FFGRAPH_TCL_PREFIX}/lib/tcl8.6" "${FFGRAPH_TK_PREFIX}/lib/tk8.6"
  DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${FFGRAPH_TCL_PREFIX}/share/licenses/tcl/license.terms"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ffgraph RENAME tcl-license.terms OPTIONAL)
install(FILES "${FFGRAPH_TK_PREFIX}/share/licenses/tk/license.terms"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ffgraph RENAME tk-license.terms OPTIONAL)
