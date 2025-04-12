file(GLOB_RECURSE SDL_EXAMPLE_1_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-1 ${SDL_EXAMPLE_1_FILES})

target_link_libraries(sdl-example-1 PRIVATE asciirast)
target_link_libraries(sdl-example-1 PRIVATE tiny_obj_loader)

target_include_directories(sdl-example-1 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-1 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-1 PRIVATE SDL2::SDL2)
