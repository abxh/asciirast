file(GLOB_RECURSE EXAMPLE_1_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(example-1 ${EXAMPLE_1_FILES})

target_link_libraries(example-1 PRIVATE asciirast)
target_link_libraries(example-1 PRIVATE examples_common)
target_link_libraries(example-1 PRIVATE terminal_utils)

target_include_directories(example-1 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(example-1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
