file(GLOB COMMON_SRCS lib_*.cc worker_common.cc)

set(TESTS
  "cpu_peak_bandwidth"
  "cpu_idle_latency"
  "cpu_loaded_latency"
  "cpu_memcpy"
  "cpu_branch_throughput"
  "cpu_numa_info"
)

foreach(TEST ${TESTS})
  if (${TEST} STREQUAL "cpu_loaded_latency")
    file(GLOB WORKER_SRCS kernels_delay_bandwidth/kernels_delay_bandwidth_*.cc)
  else()
    set(WORKER_SRCS "")
  endif()

  add_executable(${TEST} ${TEST}.cc ${COMMON_SRCS} ${WORKER_SRCS})

  if (APPLE)
    target_link_libraries(${TEST} LINK_PUBLIC
      MmMemSys
      boost_program_options pthread)
  else()
    target_link_libraries(${TEST} LINK_PUBLIC
      MmMemSys
      boost_program_options numa pthread)
  endif()

  install(TARGETS ${TEST} DESTINATION "bin")
endforeach()
