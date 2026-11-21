#include "../../windowIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)

WindowImpl::WindowImpl()
{
	DebugLog(
		"{}: Window constructor called",
		__FUNCTION__
	);

	Clear();
}

WindowImpl::~WindowImpl()
{
	DebugLog(
		"{}: Window destructor called",
		__FUNCTION__
	);

	Release();
}

b8 WindowImpl::Clear()
{
	m_width		= 0;
	m_height	= 0;
#if defined(FW_SDL3)
	m_pWindow	= nullptr;
	m_flags		= 0;
#endif	// FW_SDL3

	DebugLog(
		"{}: Window class variables cleared",
		__FUNCTION__
	);

	return true;
}

// Destroys the SDL window if it exists, then clears the state.
b8 WindowImpl::Release()
{
#if defined(FW_SDL3)
	if (m_pWindow)
	{
		SDL_DestroyWindow(m_pWindow);
	}
#endif	// FW_SDL3

	DebugLog(
		"{}: Window class resources released",
		__FUNCTION__
	);

	Clear();

	return true;
}

// Creates the SDL window. SDL owns its memory.
// If a window already exists, it fails and changes nothing.
b8 WindowImpl::CreateAppWindow(const WindowCreationDesc& desc)
{
#if defined(FW_SDL3)
	if (m_pWindow)
	{
		DebugLogWarning(
			"{}: Window is already created, skipping",
			__FUNCTION__
		);

		return false;
	}

	// SDL owns the window memory
	m_pWindow = SDL_CreateWindow(desc.m_pTitle, desc.m_width, desc.m_height, desc.m_flags);
	if (!m_pWindow)
	{
		DebugLogError(
			"{}: SDL_CreateWindow failed: {}",
			__FUNCTION__,
			SDL_GetError()
		);

		return false;
	}

	m_width		= desc.m_width;
	m_height	= desc.m_height;
	m_flags		= desc.m_flags;
#endif	// FW_SDL3

	DebugLog(
		"{}: App window is created successfully",
		__FUNCTION__
	);

	return true;
}

// Returns the SDL_Window* with SDL3. Returns nullptr without a framework.
void* WindowImpl::GetNativeWindowHandle() const
{
#if defined(FW_SDL3)
	return m_pWindow;
#else
	return nullptr;
#endif	// FW_SDL3
}

#endif	// OS_WINDOWS && HW_X64
