file(GLOB_RECURSE SDL_EXAMPLE_5_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(sdl-example-5 ${SDL_EXAMPLE_5_FILES})

target_link_libraries(sdl-example-5 PRIVATE asciirast)
target_link_libraries(sdl-example-5 PRIVATE examples_common)
target_link_libraries(sdl-example-5 PRIVATE tinyobjloader_impl)
target_link_libraries(sdl-example-5 PRIVATE tinyfd)

target_include_directories(sdl-example-5 PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(sdl-example-5 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET SDL2::SDL2main)
    target_link_libraries(sdl-example-5 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(sdl-example-5 PRIVATE SDL2::SDL2)
