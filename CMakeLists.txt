cmake_minimum_required(VERSION 3.0)

project(MM_MEM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)

add_subdirectory(cpu_micro)
