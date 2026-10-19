add_library(MmUtils
    kmg_parser.cc
    timing.cc
)

add_library(MmMemSys
    mem_region.cc
    numa_config.cc
)
target_link_libraries(MmMemSys
    MmUtils
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(MmMemSys PUBLIC ${NUMA_INCLUDE_DIR})
    target_link_libraries(MmMemSys ${NUMA_LIBRARY})
endif()
