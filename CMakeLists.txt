cmake_minimum_required(VERSION 3.21)
project(asciirast VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS True)

	if (NOT MSVC)
		if (CMAKE_BUILD_TYPE STREQUAL "Release")
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
			add_compile_options(-Ofast)
		elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			add_compile_options(-ggdb3 -Ofast)
		elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
			add_compile_options(-ggdb3)

			include(CheckCXXCompilerFlag)

			check_cxx_compiler_flag("${SANITIZER_FLAGS_ASAN}" COMPILER_SUPPORTS_ASAN)
			if (COMPILER_SUPPORTS_ASAN)
				add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
				add_link_options(-fsanitize=address)
			endif()

			check_cxx_compiler_flag("${SANITIZER_FLAGS_UBSAN}" COMPILER_SUPPORTS_UBSAN)
			if (COMPILER_SUPPORTS_UBSAN)
				add_compile_options(-fsanitize=undefined)
				add_link_options(-fsanitize=undefined)
			endif()

			# note: seems to require disabling of ASLR. So keeping TSAN off

			# check_cxx_compiler_flag("${SANITIZER_FLAGS_TSAN}" COMPILER_SUPPORTS_TSAN)
			# if (COMPILER_SUPPORTS_TSAN)
			# 	add_compile_options(-fsanitize=thread)
			# 	add_link_options(-fsanitize=thread)
			# endif()
		endif()

		# Speed up compilation if possible:
		# ---------------------------------------------------------------------------------

		find_program(MOLD_LINKER mold)
		if (MOLD_LINKER)
			message(STATUS "Found mold linker: ${MOLD_LINKER}")
			add_link_options("-fuse-ld=mold")
		else()
			message(STATUS "Mold linker not found. Using default linker")
		endif()

		find_program(CCACHE_FOUND ccache)
		if(CCACHE_FOUND)
			message(STATUS "Found ccache: ${CCACHE_FOUND}")
			set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		else()
			message(STATUS "ccache not found. Proceeding normally")
		endif()
	else ()
		if (CMAKE_BUILD_TYPE STREQUAL "Release")
			add_compile_options(/O2 /Oi)
		elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			add_compile_options(/Zi /O2)
		elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
			add_compile_options(/Zi /Od)
			add_compile_options(/analyze)
			add_compile_options(/RTC1 /RTCu /GS)
		endif()
	endif ()
endif()

# Dependencies:
# ---------------------------------------------------------------------------------

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_subdirectory(external/libassert) 
endif()

add_subdirectory(external/boost_pfr) 
add_subdirectory(external/gcem) 
add_subdirectory(external/stb_image) 

add_subdirectory(asciirast)

# Examples:
# ---------------------------------------------------------------------------------

if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)

	find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
	find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
	find_package(SDL2_ttf REQUIRED CONFIG COMPONENTS SDL2_ttf)

	add_subdirectory(external/imgui)
	add_subdirectory(external/tiny_obj_loader)
	add_subdirectory(external/tinyfiledialogs)

	if (MSVC)
		add_compile_options(/W3)
	else()
		add_compile_options(-Wall -Wextra -Wshadow -Wfloat-conversion -Wsign-conversion -Warith-conversion -pedantic)
	endif()

	add_subdirectory(examples/common)
	add_subdirectory(examples/00-spiral)
	add_subdirectory(examples/01-sierpinski-triangle)
	add_subdirectory(examples/02-wheel)

	add_subdirectory(examples/IMG-00-perspective-correctness)
	add_subdirectory(examples/IMG-01-mipmapping)

	add_subdirectory(examples/SDL-00-hello-triangle)
	add_subdirectory(examples/SDL-01-wireframe)
	add_subdirectory(examples/SDL-02-flat-shading)
	add_subdirectory(examples/SDL-03-shading-with-depth)
	add_subdirectory(examples/SDL-04-texture-sampling)
	add_subdirectory(examples/SDL-05-shading-with-color)
	# add_subdirectory(examples/SDL-06-camera)
endif()
