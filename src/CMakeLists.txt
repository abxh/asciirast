file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(asciirast SHARED ${LIB_FILES})

target_include_directories(asciirast PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(asciirast PUBLIC stb_image)
