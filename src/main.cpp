#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void** AppState, int, char**)
{
    // ...
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* AppState, SDL_Event* Event)
{
    // ...
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* AppState)
{
    // ...
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* AppState, SDL_AppResult Result)
{
    // ...
}