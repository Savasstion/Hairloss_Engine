#include "../../windowIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)

WindowImpl::WindowImpl()
{
	Release();
}

WindowImpl::~WindowImpl()
{
	Release();
}

void WindowImpl::Clear()
{
	m_width		= 0;
	m_height	= 0;
#if defined(FW_SDL3)
	m_flags		= 0;
#endif	// FW_SDL3
}

void WindowImpl::Release()
{
#if defined(FW_SDL3)
	m_pWindow.reset();
#endif	// FW_SDL3

	Clear();
}

b8 WindowImpl::CreateAppWindow(const WindowCreationDesc& desc)
{
#if defined(FW_SDL3)
	SDL_Window* pWindowRawPtr = SDL_CreateWindow(desc.m_pTitle, desc.m_width, desc.m_height, desc.m_flags);
	if (!pWindowRawPtr)
	{
		// TODO : Use own logging function
		SDL_Log(
			"%s: SDL_CreateWindow failed: %s",
			__FUNCTION__,
			SDL_GetError()
		);

		return false;
	}

	// Assign shared pointer and custom deletor
	m_pWindow = std::shared_ptr<SDL_Window>(pWindowRawPtr, [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });

#endif	// FW_SDL3
	return true;
}

#endif