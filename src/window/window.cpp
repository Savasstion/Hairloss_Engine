#include "windowIncludes.h"

// Allocates the WindowImpl from pAllocator. It does not create the OS window. To create it, call CreateAppWindow().
// The window keeps pAllocator but does not own it. pAllocator must stay alive longer than the window.
// Without an allocator, it logs and keeps the impl as nullptr. Then, later calls fail safely.
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

// Deletes the WindowImpl. The WindowImpl destructor destroys the SDL window.
// Call it before you release the heap of the allocator. If not, the SDL window leaks.
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
// Fails if the impl is not allocated or if a window already exists.
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

// Returns the type-erased framework window (SDL_Window* with SDL3). Returns nullptr if there is no window.
// The caller casts it back to the framework type inside its framework guard.
void* Window::GetNativeWindowHandle() const
{
	return m_pWindowImpl ? m_pWindowImpl->GetNativeWindowHandle() : nullptr;
}
