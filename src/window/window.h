#pragma once

class Window 
{
private:
	std::shared_ptr<WindowImpl> m_pWindowImpl;
public:
	Window();
	~Window();
	void Clear();
	void Release();
};