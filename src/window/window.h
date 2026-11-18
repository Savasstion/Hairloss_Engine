#pragma once

class Window
{
public:
	CpuHeapAllocator* m_pAllocator;		// not owned
	WindowImpl* m_pWindowImpl;

	Window(CpuHeapAllocator* pAllocator);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	b8 Clear();
	b8 Release();

	b8 CreateAppWindow(const WindowCreationDesc& desc);
	void* GetNativeWindowHandle() const;
};
