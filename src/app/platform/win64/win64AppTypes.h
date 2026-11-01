#pragma once

//  Forward declarations
class Window;

struct AppState
{
    std::shared_ptr<Window> m_pWindow;
#if defined(FW_SDL3)
    SDL_Renderer* m_pRenderer = nullptr;
#endif  //  FW_SDL3
};