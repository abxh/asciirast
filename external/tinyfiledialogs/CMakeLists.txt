file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(tinyfiledialogs SHARED ${LIB_FILES})

target_include_directories(tinyfiledialogs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
