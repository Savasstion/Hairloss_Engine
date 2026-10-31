#pragma once

//	Framework types
#if defined(FW_SDL3)
struct AppState
{
    std::shared_ptr<Window> m_pWindow;
    SDL_Renderer* m_pRenderer = nullptr;
};
#endif