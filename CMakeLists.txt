cmake_minimum_required(VERSION 3.16)

project(MM_MEM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_path(NUMA_INCLUDE_DIR numa.h HINTS "$ENV{CONDA_PREFIX}/include")
    find_library(NUMA_LIBRARY numa HINTS "$ENV{CONDA_PREFIX}/lib")
    if(NOT NUMA_INCLUDE_DIR OR NOT NUMA_LIBRARY)
        message(FATAL_ERROR "libnuma headers/library missing. Run scripts/install_deps_sudo.sh or scripts/install_deps_conda.sh.")
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)

add_subdirectory(cpu_micro)
