file(GLOB_RECURSE SDL_EXAMPLE_4_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-4 ${SDL_EXAMPLE_4_FILES})

target_link_libraries(sdl-example-4 PRIVATE asciirast)
target_link_libraries(sdl-example-4 PRIVATE tiny_obj_loader)

target_include_directories(sdl-example-4 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-4 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-4 PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
