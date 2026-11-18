#include "windowIncludes.h"

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

b8 Window::Release()
{
	if (m_pAllocator)
	{
		m_pAllocator->DeleteObject(m_pWindowImpl);
	}

	Clear();

	return true;
}

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

void* Window::GetNativeWindowHandle() const
{
	return m_pWindowImpl ? m_pWindowImpl->GetNativeWindowHandle() : nullptr;
}
