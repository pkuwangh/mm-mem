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
