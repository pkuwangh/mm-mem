file(GLOB COMMON_SRCS lib_*.cc)

set(TESTS
  "test_peak_bandwidth"
)

foreach(TEST ${TESTS})
  add_executable(${TEST} ${TEST}.cc ${COMMON_SRCS})
  target_link_libraries(${TEST} boost_program_options numa pthread)
endforeach()

