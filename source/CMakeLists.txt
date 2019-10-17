cmake_minimum_required(VERSION 3.0)
enable_testing()

SET(CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -Wextra")

project(Heap C ASM)

include(cmake/Unittests.cmake)
add_subdirectory(unittest_framework)
add_subdirectory(heap)