cmake_minimum_required(VERSION 3.12)
project(asciirast VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wfloat-conversion -Wsign-conversion -Warith-conversion -pedantic")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)

set(SDL_BASED_EXAMPLES ON)

if (SDL_BASED_EXAMPLES)
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

add_subdirectory(external/terminal_utils)
# add_subdirectory(external/tiny_obj_loader)
# add_subdirectory(external/stb_image)

add_subdirectory(asciirast)

add_subdirectory(examples/00-spiral)
add_subdirectory(examples/01-sierpinski-triangle)

if (SDL_BASED_EXAMPLES)
	add_subdirectory(examples/02-hello-triangle)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
endif()

# set(CMAKE_BUILD_TYPE "Debug")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")

	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("${SANITIZER_FLAGS_ASAN}" COMPILER_SUPPORTS_ASAN)
	check_cxx_compiler_flag("${SANITIZER_FLAGS_UBSAN}" COMPILER_SUPPORTS_UBSAN)
	check_cxx_compiler_flag("${SANITIZER_FLAGS_UBSAN}" COMPILER_SUPPORTS_TSAN)

	if(COMPILER_SUPPORTS_ASAN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
	else()
		message(WARNING "Address sanitizer was not found!")
	endif()
	if(COMPILER_SUPPORTS_UBSAN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
	else()
		message(WARNING "UB sanitizer was not found!")
	endif()
	# note: seems to require disabling of ASLR. So keeping this off
	# if(COMPILER_SUPPORTS_TSAN)
	# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
	# 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
	# else()
	# 	message(WARNING "Thread sanitizer was not found!")
	# endif()
endif()
