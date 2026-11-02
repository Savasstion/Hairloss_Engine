#include "../../windowIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)

WindowImpl::WindowImpl()
{
	DebugLog(
		"{}: Window constructor called",
		__FUNCTION__
	);

	Release();
}

WindowImpl::~WindowImpl()
{
	DebugLog(
		"{}: Window destructor called",
		__FUNCTION__
	);

	Release();
}

void WindowImpl::Clear()
{
	m_width		= 0;
	m_height	= 0;
#if defined(FW_SDL3)
	m_flags		= 0;
#endif	// FW_SDL3

	DebugLog(
		"{}: Window class variables cleared",
		__FUNCTION__
	);
}

void WindowImpl::Release()
{
#if defined(FW_SDL3)
	m_pWindow.reset();
#endif	// FW_SDL3

	DebugLog(
		"{}: Window class resources released",
		__FUNCTION__
	);

	Clear();
}

b8 WindowImpl::CreateAppWindow(const WindowCreationDesc& desc)
{
#if defined(FW_SDL3)
	// TODO : Check how does this allocate in memory and see if could own heap allocation
	SDL_Window* pWindowRawPtr = SDL_CreateWindow(desc.m_pTitle, desc.m_width, desc.m_height, desc.m_flags);
	if (!pWindowRawPtr)
	{
		// TODO : Use own logging function
		DebugLogError(
			"{}: SDL_CreateWindow failed: {}",
			__FUNCTION__,
			SDL_GetError()
		);

		return false;
	}

	// Assign shared pointer and custom deleter
	m_pWindow = std::shared_ptr<SDL_Window>(pWindowRawPtr, [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });

#endif	// FW_SDL3

	DebugLog(
		"{}: App window is created successfully",
		__FUNCTION__
	);

	return true;
}

#endif