# Extra file for build support of sdl backend

file(GLOB CURRENT_DIR_CPP CONFIGURE_DEPENDS *.cpp)

add_library(imgui_sdl2 SHARED
    ${CURRENT_DIR_CPP}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_src/backends/imgui_impl_sdl2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_src/backends/imgui_impl_sdlrenderer2.cpp)

target_include_directories(imgui_sdl2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui_src)

if(TARGET SDL2::SDL2main)
    target_link_libraries(imgui_sdl2 PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(imgui_sdl2 PRIVATE SDL2::SDL2)
