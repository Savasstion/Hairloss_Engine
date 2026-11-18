#pragma once

struct AppState
{
	// Main app memory
	CpuHeap m_appHeap;
	CpuHeapBumpAllocator m_appAllocator;

	// App objects
	Window* m_pWindow = nullptr;

	// App main systems
#if defined(FW_SDL3)
	// TODO : Make own Graphic System
	SDL_Renderer* m_pRenderer = nullptr;
#endif	// FW_SDL3
};
