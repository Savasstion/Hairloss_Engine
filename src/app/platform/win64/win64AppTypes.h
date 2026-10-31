#pragma once

//	Framework types
#if defined(FW_SDL3)
struct AppState
{
    SDL_Window* m_pWindow = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};
#endif