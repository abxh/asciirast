add_library(examples_common INTERFACE)

target_include_directories(examples_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(examples_common INTERFACE terminal_utils)

if(TARGET SDL2::SDL2main)
    target_link_libraries(examples_common INTERFACE SDL2::SDL2main)
endif()

target_link_libraries(examples_common INTERFACE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
