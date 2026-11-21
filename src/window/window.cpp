#include "windowIncludes.h"

// Allocates the WindowImpl from pAllocator, no OS window yet, call CreateAppWindow()
// pAllocator is kept, not owned, and must outlive the window
// Without an allocator, logs and leaves the impl nullptr, so later calls fail cleanly
Window::Window(CpuHeapAllocator* pAllocator)
{
	Clear();

	m_pAllocator = pAllocator;
	if (!m_pAllocator)
	{
		DebugLogError(
			"{}: Window needs an allocator",
			__FUNCTION__
		);

		return;
	}

	m_pWindowImpl = m_pAllocator->NewObject<WindowImpl>();
}

Window::~Window()
{
	Release();
}

b8 Window::Clear()
{
	m_pAllocator	= nullptr;
	m_pWindowImpl	= nullptr;

	return true;
}

// Deletes the WindowImpl, whose destructor destroys the SDL window
// Must run before the allocator's heap is released, the SDL window leaks otherwise
b8 Window::Release()
{
	if (m_pAllocator)
	{
		m_pAllocator->DeleteObject(m_pWindowImpl);
	}

	Clear();

	return true;
}

// Creates the OS window from desc through the impl
// Fails if the impl wasn't allocated or a window already exists
b8 Window::CreateAppWindow(const WindowCreationDesc& desc)
{
	if (!m_pWindowImpl)
	{
		DebugLogError(
			"{}: Window implementation was not allocated",
			__FUNCTION__
		);

		return false;
	}

	return m_pWindowImpl->CreateAppWindow(desc);
}

// Type-erased framework window (SDL_Window* with SDL3), nullptr without one
// Callers cast it back inside their own framework guard
void* Window::GetNativeWindowHandle() const
{
	return m_pWindowImpl ? m_pWindowImpl->GetNativeWindowHandle() : nullptr;
}
