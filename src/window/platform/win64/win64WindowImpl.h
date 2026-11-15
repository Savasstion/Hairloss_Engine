#pragma once

class WindowImpl 
{
public:
	u32 m_width;
	u32 m_height;
#if defined(FW_SDL3)
	std::shared_ptr<SDL_Window> m_pWindow;
	SDL_WindowFlags m_flags;
#endif	// FW_SDL3

	WindowImpl();
	~WindowImpl();
	b8 Clear();
	b8 Release();

	b8 CreateAppWindow(const WindowCreationDesc& desc);
	void* GetNativeWindowHandle() const;
};
