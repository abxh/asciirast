find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND)
 
  CPMAddPackage(
    NAME SDL2
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-2.28.5
    OPTIONS "SDL_STATIC=OFF"
  )
endif()

find_package(SDL2_ttf CONFIG QUIET)

if(NOT SDL2_ttf_FOUND)
 
  CPMAddPackage(
    NAME SDL2_ttf
    GITHUB_REPOSITORY libsdl-org/SDL_ttf
    GIT_TAG release-2.22.0
    OPTIONS "SDL2TTF_SAMPLES=OFF"
  )
endif()
