cmake_minimum_required(VERSION 3.12)
project(asciirast VERSION 0.1)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -pedantic")

# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(CMAKE_CXX_FLAGS "-fsanitize=address -fsanitize=undefined -ggdb3")
set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address -fsanitize=undefined")
# endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)

add_subdirectory(external)
add_subdirectory(asciirast)
add_subdirectory(adapters)
