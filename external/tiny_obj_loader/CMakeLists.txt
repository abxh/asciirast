file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(tiny_obj_loader STATIC ${LIB_FILES})

target_include_directories(tiny_obj_loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
