#pragma once

class Window 
{
public:
	std::shared_ptr<WindowImpl> m_pWindowImpl;

	Window();
	~Window();
	b8 Clear();
	b8 Release();

	b8 CreateAppWindow(const WindowCreationDesc& desc);
};