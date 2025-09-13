file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(tinyobjloader_impl SHARED ${LIB_FILES})

target_link_libraries(tinyobjloader_impl PUBLIC tinyobjloader)
