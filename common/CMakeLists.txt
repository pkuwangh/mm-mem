add_library(Timing timing.cc)
add_library(KmgParser kmg_parser.cc)
add_library(MemRegion mem_region.cc)

install(TARGETS Timing DESTINATION "lib")
install(TARGETS KmgParser DESTINATION "lib")
install(TARGETS MemRegion DESTINATION "lib")
