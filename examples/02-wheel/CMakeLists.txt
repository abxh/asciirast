file(GLOB_RECURSE GLOBBED_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(example-2 ${GLOBBED_FILES})

target_link_libraries(example-2 PRIVATE asciirast)
target_link_libraries(example-2 PRIVATE examples_common)

target_include_directories(example-2 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(example-2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
