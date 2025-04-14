add_library(asciirast INTERFACE)
target_include_directories(asciirast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(asciirast INTERFACE stb_image)
