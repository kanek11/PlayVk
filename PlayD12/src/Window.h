#pragma once

#include "Base.h"

#include "InputSystem.h"

//abstraction of window. 
//wrapper of win32 API by modern C++
  
struct WindowCreateInfo
{
	int width;
	int height;
	std::string title;
	//bool resizable;
};


class WindowBase {
public:
	WindowBase() = default;
	virtual ~WindowBase() = default;

	virtual bool shouldClose() = 0;
	virtual void onUpdate() = 0; 

	virtual void* GetRawHandle() = 0;

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0; 
	 
	virtual void SetCustomWindowText(const std::string& text) = 0;
	//void InitInputSource(InputSystem* inputsystem) {
	//	inputSource = new WindowsInputSource(inputsystem);
	//}

public:
	WindowsInputSource* inputSource =  new WindowsInputSource();
};

//factory:
class WindowFactory {
public: 
	 static SharedPtr<WindowBase> createWindow(const WindowCreateInfo& createInfo); 
};
 
class Win32Window : public WindowBase {
public:
	Win32Window(const WindowCreateInfo& createInfo);
	~Win32Window() = default;

	virtual bool shouldClose() override;
	virtual void onUpdate() override;

	virtual void* GetRawHandle() override { return m_hwnd; }
	virtual int GetWidth() const override { return m_width; }
	virtual int GetHeight() const override { return m_height; } 

public:
	void SetCustomWindowText(const std::string& text) override;

private:
	void pollEvents(); 


private:
	int m_width = 1280; 
	int m_height = 720;  

private:
	HINSTANCE m_hInstance = nullptr; // Handle to the process instance
	HWND m_hwnd = nullptr; // Handle to the window

	MSG m_msg = {};

};