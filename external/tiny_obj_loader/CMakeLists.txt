file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(tiny_obj_loader SHARED ${LIB_FILES})

target_include_directories(tiny_obj_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
