file(GLOB_RECURSE EXAMPLE_0_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(example-0 ${EXAMPLE_0_FILES})

target_link_libraries(example-0 PRIVATE asciirast)
target_link_libraries(example-0 PRIVATE terminal_utils)

target_include_directories(example-0 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(example-0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
