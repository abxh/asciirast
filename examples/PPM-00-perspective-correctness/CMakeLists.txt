file(GLOB_RECURSE GLOBBED_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(ppm-example-0 ${GLOBBED_FILES})

target_link_libraries(ppm-example-0 PRIVATE asciirast)
target_link_libraries(ppm-example-0 PRIVATE terminal_utils)

target_include_directories(ppm-example-0 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(ppm-example-0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
