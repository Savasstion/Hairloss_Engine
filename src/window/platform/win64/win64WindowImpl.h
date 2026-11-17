#pragma once

class WindowImpl 
{
public:
	u32 m_width;
	u32 m_height;
#if defined(FW_SDL3)
	SDL_Window* m_pWindow;		// owned by SDL
	SDL_WindowFlags m_flags;
#endif	// FW_SDL3

	WindowImpl();
	~WindowImpl();
	b8 Clear();
	b8 Release();

	b8 CreateAppWindow(const WindowCreationDesc& desc);
	void* GetNativeWindowHandle() const;
};
