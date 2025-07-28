file(GLOB_RECURSE GLOBBED_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(img-example-1 ${GLOBBED_FILES})

target_link_libraries(img-example-1 PRIVATE asciirast)
target_link_libraries(img-example-1 PRIVATE examples_common)
target_link_libraries(img-example-1 PRIVATE tinyfiledialogs)

target_include_directories(img-example-1 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(img-example-1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
