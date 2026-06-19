set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

add_subdirectory(SDL)
add_subdirectory(Catch2)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE
    SDL3::SDL3-static
)

add_library(tests INTERFACE)
target_link_libraries(vendor INTERFACE
    Catch2::Catch2WithMain
)