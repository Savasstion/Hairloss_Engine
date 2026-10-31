#pragma once

class WindowImpl 
{
private:
	std::shared_ptr<SDL_Window> m_pWindow;
	u32 m_width;
	u32 m_height;
	SDL_WindowFlags m_flags;
public:
	WindowImpl(const char* pTitle = pWindowName, const u32 width, const u32 height, const SDL_WindowFlags flags);
	~WindowImpl();
	void Clear();
	void Release();
};