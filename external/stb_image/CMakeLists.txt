file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(stb_image_shared SHARED ${LIB_FILES})

target_include_directories(stb_image_shared PUBLIC ${stb_SOURCE_DIR})
