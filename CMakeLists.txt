cmake_minimum_required(VERSION 3.12)
project(asciirast VERSION 0.1)

set(EXAMPLES ON)
set(SDL_BASED_EXAMPLES ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-DNDEBUG -O3 -ffast-math -ffinite-math-only -flto=auto) # alternatively: -Ofast
endif()

if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	add_compile_options(-ggdb3)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-ggdb3)

	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("${SANITIZER_FLAGS_ASAN}" COMPILER_SUPPORTS_ASAN)
	check_cxx_compiler_flag("${SANITIZER_FLAGS_UBSAN}" COMPILER_SUPPORTS_UBSAN)
	# check_cxx_compiler_flag("${SANITIZER_FLAGS_UBSAN}" COMPILER_SUPPORTS_TSAN)

	if(COMPILER_SUPPORTS_ASAN)
		add_compile_options(-fsanitize=address)
		add_link_options(-fsanitize=address)
	endif()
	if(COMPILER_SUPPORTS_UBSAN)
		add_compile_options(-fsanitize=undefined)
		add_link_options(-fsanitize=undefined)
	endif()
	# note: seems to require disabling of ASLR. So keeping TSAN off
	# if(COMPILER_SUPPORTS_TSAN)
	# 	add_compile_options(-fsanitize=thread)
	# 	add_link_options(-fsanitize=thread)
	# endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)

# neccessary dependencies for asciirast:
add_subdirectory(external/stb_image) 

add_subdirectory(asciirast)

if (EXAMPLES)
	if (SDL_BASED_EXAMPLES)
		find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
		find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
		find_package(SDL2_ttf REQUIRED CONFIG COMPONENTS SDL2_ttf)
		# add_subdirectory(external/imgui)
	endif()

	add_subdirectory(external/tiny_obj_loader)

	if (MSVC)
		add_compile_options(/W3)
	else()
		add_compile_options(-Wall -Wextra -Wshadow -Wfloat-conversion -Wsign-conversion -Warith-conversion -pedantic)
	endif()

	add_subdirectory(external/terminal_utils)

	add_subdirectory(examples/00-spiral)
	add_subdirectory(examples/01-sierpinski-triangle)
	add_subdirectory(examples/02-wheel)

	if (SDL_BASED_EXAMPLES)
		add_subdirectory(examples/SDL-00-hello-triangle)
		add_subdirectory(examples/SDL-01-wireframe)
		add_subdirectory(examples/SDL-02-flat-shading)
		add_subdirectory(examples/SDL-03-shading-with-depth)
		add_subdirectory(examples/SDL-04-texture-sampling)
		add_subdirectory(examples/SDL-05-shading-with-color)
		# add_subdirectory(examples/SDL-06-camera)
	endif()

	add_subdirectory(examples/IMG-00-perspective-correctness)
	add_subdirectory(examples/IMG-01-mipmapping)
endif()
