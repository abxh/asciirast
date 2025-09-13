cmake_minimum_required(VERSION 3.21)
project(asciirast VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS True)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
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

message(STATUS "Bootstrapping CPM.cmake")
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
set(CPM_USE_LOCAL_PACKAGES TRUE)

CPMAddPackage("gh:boostorg/pfr#master")
if (TARGET boost_pfr)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(boost_pfr INTERFACE -Wno-shadow)
  endif()
endif()

CPMAddPackage(
  NAME gcem
  GIT_REPOSITORY https://github.com/kthohr/gcem.git
  GIT_TAG master
  DOWNLOAD_ONLY TRUE
)
add_library(gcem INTERFACE)
target_include_directories(gcem INTERFACE ${gcem_SOURCE_DIR}/include)

CPMAddPackage(
  NAME stb
  GITHUB_REPOSITORY nothings/stb
  GIT_TAG master
  DOWNLOAD_ONLY TRUE
)
add_subdirectory(external/stb_image_impl)

add_subdirectory(include)

# Examples:
# ---------------------------------------------------------------------------------

if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/asciirast)

	CPMAddPackage(
	  NAME SDL2
	  GITHUB_REPOSITORY libsdl-org/SDL
	  GIT_TAG SDL2
	)

	CPMAddPackage(
	  NAME tinyobjloader
	  GITHUB_REPOSITORY tinyobjloader/tinyobjloader
	  GIT_TAG release
	)
	add_subdirectory(external/tinyobjloader_impl)

	CPMAddPackage(
	  NAME tinyfiledialogs
	  GIT_REPOSITORY http://git.code.sf.net/p/tinyfiledialogs/code
	  GIT_TAG master
	  DOWNLOAD_ONLY TRUE
	)
	add_library(tinyfd STATIC
	  ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
	)
	target_include_directories(tinyfd PUBLIC ${tinyfiledialogs_SOURCE_DIR})

	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra -Wshadow -Wfloat-conversion -Wsign-conversion -Warith-conversion -pedantic)
		add_compile_options(-fconcepts-diagnostics-depth=10)
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		add_compile_options(/W3)
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
