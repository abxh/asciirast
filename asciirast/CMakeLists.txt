add_library(asciirast INTERFACE)

target_include_directories(asciirast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(asciirast INTERFACE libassert::assert)
endif()

target_link_libraries(asciirast INTERFACE boost_pfr)
target_link_libraries(asciirast INTERFACE gcem)
target_link_libraries(asciirast INTERFACE stb_image)
