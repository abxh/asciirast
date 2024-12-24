file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp math/*.cpp)

add_library(asciirast SHARED ${LIB_FILES})

target_include_directories(asciirast PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(asciirast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
