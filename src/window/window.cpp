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

void Window::Clear()
{

}

void Window::Release()
{
	m_pWindowImpl.reset();

	Clear();
}