file(GLOB_RECURSE GLOBBED_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(img-example-0 ${GLOBBED_FILES})

target_link_libraries(img-example-0 PRIVATE asciirast)

target_include_directories(img-example-0 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(img-example-0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
