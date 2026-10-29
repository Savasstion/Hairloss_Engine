#pragma once

//	Framework types
#if defined(FW_SDL3)
struct AppState
{
    SDL_Window* pWindow = nullptr;
    SDL_Renderer* pRenderer = nullptr;
};
#endif