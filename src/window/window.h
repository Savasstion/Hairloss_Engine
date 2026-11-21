#pragma once

// Platform-agnostic window, forwards to WindowImpl
// m_pAllocator: not owned, must outlive the window
// m_pWindowImpl: owned, allocated from m_pAllocator
class Window
{
public:
	CpuHeapAllocator* m_pAllocator;
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
