#pragma once

// Default window defines
constexpr const char* pDefaultWindowTitle = "Hairloss Engine";
constexpr u32 defaultWindowWidth = 1920;
constexpr u32 defaultWindowHeight = 1080;

#if defined(FW_SDL3)
constexpr SDL_WindowFlags defaultWindowFlags = SDL_WINDOW_RESIZABLE;
#endif