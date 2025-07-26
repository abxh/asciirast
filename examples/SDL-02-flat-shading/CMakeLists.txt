file(GLOB_RECURSE SDL_EXAMPLE_2_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-2 ${SDL_EXAMPLE_2_FILES})

target_link_libraries(sdl-example-2 PRIVATE asciirast)
target_link_libraries(sdl-example-2 PRIVATE tiny_obj_loader)
target_link_libraries(sdl-example-2 PRIVATE tinyfiledialogs)

target_include_directories(sdl-example-2 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-2 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-2 PRIVATE SDL2::SDL2)
