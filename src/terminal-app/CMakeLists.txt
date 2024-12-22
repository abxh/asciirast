file(GLOB_RECURSE TERMINAL_APP_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(terminal-app ${TERMINAL_APP_FILES})

target_link_libraries(terminal-app asciirast)

target_include_directories(terminal-app PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(terminal-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
