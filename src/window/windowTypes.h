#pragma once

struct WindowCreationDesc 
{
	u32 m_width;
	u32 m_height;
	const char* m_pTitle;
#if defined(FW_SDL3)
	SDL_WindowFlags m_flags;
#endif
};