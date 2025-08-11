file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS *.cpp)

add_library(examples_common STATIC ${LIB_FILES})

target_include_directories(examples_common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(examples_common PRIVATE asciirast)

if(TARGET SDL2::SDL2main)
    target_link_libraries(examples_common PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(examples_common PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
