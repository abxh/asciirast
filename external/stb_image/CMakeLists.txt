file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(stb_image SHARED ${LIB_FILES})

target_include_directories(stb_image PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
