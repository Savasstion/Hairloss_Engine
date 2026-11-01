#pragma once

#include "appIncludes.h"

struct AppState
{
    std::shared_ptr<Window> m_pWindow;
#if defined(FW_SDL3)
    // TODO : Make own Graphic System
    SDL_Renderer* m_pRenderer = nullptr;
#endif  //  FW_SDL3
};