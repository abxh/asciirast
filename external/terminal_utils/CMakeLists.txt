file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(terminal_utils STATIC ${LIB_FILES})

target_include_directories(terminal_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
