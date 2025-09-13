file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(tiny_obj_loader SHARED ${LIB_FILES})

target_link_libraries(tiny_obj_loader PUBLIC tinyobjloader)
