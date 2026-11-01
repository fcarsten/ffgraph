# Check the helper's failure path in a child process, so CTest can distinguish
# the expected abort from a broken check that returns success under NDEBUG.
execute_process(COMMAND "${TEST_BINARY}"
  RESULT_VARIABLE success OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT "${success}" STREQUAL "0")
  message(FATAL_ERROR "NDEBUG side-effect check failed: ${success}\n${output}${error}")
endif()
execute_process(COMMAND "${TEST_BINARY}" --intentional-failure
  RESULT_VARIABLE failure OUTPUT_VARIABLE output ERROR_VARIABLE error)
if("${failure}" STREQUAL "0" OR NOT error MATCHES "check failed: false")
  message(FATAL_ERROR "NDEBUG failure check did not abort as expected: ${failure}\n${output}${error}")
endif()
