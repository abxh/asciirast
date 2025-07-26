file(GLOB_RECURSE SDL_EXAMPLE_3_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-3 ${SDL_EXAMPLE_3_FILES})

target_link_libraries(sdl-example-3 PRIVATE asciirast)
target_link_libraries(sdl-example-3 PRIVATE tiny_obj_loader)
target_link_libraries(sdl-example-3 PRIVATE tinyfiledialogs)

target_include_directories(sdl-example-3 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-3 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-3 PRIVATE SDL2::SDL2)
