file(GLOB_RECURSE SDL_EXAMPLE_0_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-0 ${SDL_EXAMPLE_0_FILES})

target_link_libraries(sdl-example-0 PRIVATE asciirast)
target_link_libraries(sdl-example-0 PRIVATE terminal_utils)

target_include_directories(sdl-example-0 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-0 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-0 PRIVATE SDL2::SDL2)
