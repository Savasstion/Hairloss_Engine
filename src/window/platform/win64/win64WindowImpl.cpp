#include "../../windowIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)

WindowImpl::WindowImpl(const char* pTitle, const u32 width, const u32 height, const SDL_WindowFlags flags)
{
	Release();

	// Create window
	SDL_Window* pWindowRawPtr = SDL_CreateWindow(pTitle, width, height, flags);
	if (!pWindowRawPtr)
	{
		// TODO : Use own logging function
		SDL_Log(
			"%s: SDL_CreateWindow failed: %s", 
			__FUNCTION__, 
			SDL_GetError()
		);
		return;
	}

	// Assign shared pointer and custom deletor
	m_pWindow = std::shared_ptr<SDL_Window>(pWindowRawPtr, [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });
}

WindowImpl::~WindowImpl()
{
	Release();
}

void WindowImpl::Clear()
{
	m_width		= 0;
	m_height	= 0;
}

void WindowImpl::Release()
{
	m_pWindow.reset();

	Clear();
}

#endif