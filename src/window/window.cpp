#include "windowIncludes.h"

Window::Window(CpuHeapAllocator* pHeapAllocator)
{
	Clear();

	m_pHeapAllocator = pHeapAllocator;
	if (!m_pHeapAllocator)
	{
		DebugLogError(
			"{}: Window needs a heap allocator",
			__FUNCTION__
		);

		return;
	}

	m_pWindowImpl = m_pHeapAllocator->New<WindowImpl>();
}

Window::~Window()
{
	Release();
}

b8 Window::Clear()
{
	m_pHeapAllocator	= nullptr;
	m_pWindowImpl		= nullptr;

	return true;
}

b8 Window::Release()
{
	if (m_pHeapAllocator)
	{
		m_pHeapAllocator->Delete(m_pWindowImpl);
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
