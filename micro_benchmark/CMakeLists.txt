file(GLOB COMMON_SRCS lib_*.cc worker_common.cc)

set(TESTS
  "test_peak_bandwidth"
  "test_idle_latency"
  "test_loaded_latency"
)

foreach(TEST ${TESTS})
  if (${TEST} STREQUAL "test_loaded_latency")
    file(GLOB WORKER_SRCS worker_kernels_delay_bandwidth_*.cc)
  else()
    set(WORKER_SRCS "")
  endif()
  add_executable(${TEST} ${TEST}.cc ${COMMON_SRCS} ${WORKER_SRCS})
  target_link_libraries(${TEST} boost_program_options numa pthread)
endforeach()

