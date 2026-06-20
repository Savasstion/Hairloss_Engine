#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

struct AppState
{
    SDL_Window*   pWindow   = nullptr;
    SDL_Renderer* pRenderer = nullptr;
};

SDL_AppResult SDL_AppInit(void** ppAppState, int, char**)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return SDL_APP_FAILURE;

    AppState* pState = new AppState();
    *ppAppState = pState;

    pState->pWindow = SDL_CreateWindow("Hairloss Engine", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!pState->pWindow)
        return SDL_APP_FAILURE;

    pState->pRenderer = SDL_CreateRenderer(pState->pWindow, nullptr);
    if (!pState->pRenderer)
        return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* pAppState, SDL_Event* pEvent)
{
    if (pEvent->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* pAppState)
{
    AppState* pState = static_cast<AppState*>(pAppState);
    
    SDL_SetRenderDrawColor(pState->pRenderer, 30, 30, 30, 255);
    SDL_RenderClear(pState->pRenderer);
    SDL_RenderPresent(pState->pRenderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* pAppState, SDL_AppResult)
{
    AppState* pState = static_cast<AppState*>(pAppState);
    if (pState)
    {
        SDL_DestroyRenderer(pState->pRenderer);
        SDL_DestroyWindow(pState->pWindow);
        delete pState;
    }

    SDL_Quit();
}