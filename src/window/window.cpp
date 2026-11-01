#include "windowIncludes.h"

Window::Window()
{
	Release();

	// TODO : Use own heap implementation to allocate memory
	m_pWindowImpl = std::make_shared<WindowImpl>();
}

Window::~Window()
{
	Release();
}

b8 Window::Clear()
{
	return true;
}

b8 Window::Release()
{
	m_pWindowImpl.reset();
	Clear();

	return true;
}

b8 Window::CreateAppWindow(const WindowCreationDesc& desc)
{
	return m_pWindowImpl->CreateAppWindow(desc);
}
