file(GLOB COMMON_SRCS lib_*.cc worker_common.cc)

set(TESTS
  "cpu_peak_bandwidth"
  "cpu_idle_latency"
  "cpu_loaded_latency"
  "cpu_memcpy"
  "cpu_branch_throughput"
)

foreach(TEST ${TESTS})

  add_executable(${TEST} ${TEST}.cc ${COMMON_SRCS})

  if (APPLE)
    target_link_libraries(${TEST} LINK_PUBLIC
      boost_program_options pthread
      Timing MemRegion)
  else()
    target_link_libraries(${TEST} LINK_PUBLIC
      boost_program_options numa pthread
      Timing MemRegion)
  endif()

  install(TARGETS ${TEST} DESTINATION "bin")
endforeach()

