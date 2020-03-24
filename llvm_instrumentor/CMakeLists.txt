cmake_minimum_required(VERSION 2.8.0)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(insert_call_etm_pass)
add_subdirectory(dynamic-rt)
