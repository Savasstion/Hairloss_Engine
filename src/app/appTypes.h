#pragma once

struct AppState
{
	// Main app memory. Use it for objects that stay alive for a long time, for example systems.
	CpuHeap m_appHeap;

	// App objects
	Window* m_pWindow = nullptr;

	// App main systems
#if defined(FW_SDL3)
	// TODO : Make own Graphic System
	SDL_Renderer* m_pRenderer = nullptr;
#endif	// FW_SDL3
};
