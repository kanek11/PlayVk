#include "PCH.h"
#include "Window.h"
 

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//special handling of WM_CREATE message to store the user data
	if (message == WM_CREATE) {
		auto cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}

	Win32Window* self = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (!self)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	switch (message)
	{
	//case WM_CREATE:
	//{ 
	//	LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
	//	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));

	//	return 0;
	//} 

	case WM_PAINT:
	{ 
		return 0;
	} 

	case WM_SIZE:
	{  
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		std::cout << "Window resized to: " << width << "x" << height << std::endl;
		return 0;
	}

	case WM_KEYDOWN:
	{  
		int keyCode = static_cast<int>(wParam);
		std::cout << "Key pressed: " << keyCode << std::endl;
		return 0;
	}

	case WM_KEYUP:
	{  
		int keyCode = static_cast<int>(wParam);
		std::cout << "Key released: " << keyCode << std::endl;
		return 0;
	} 

	case WM_MOUSEMOVE: 
	{ 
		POINTS pt = MAKEPOINTS(lParam);
		//std::cout << "Mouse moved to: (" << pt.x << ", " << pt.y << ")" << std::endl;
		return 0; 
	}

	case WM_LBUTTONDOWN:
	{ 
		std::cout << "Left button pressed" << std::endl;
		return 0;
	}

	case WM_LBUTTONUP:
	{ 
		std::cout << "Left button released" << std::endl;
		return 0;
	}

	case WM_RBUTTONDOWN:
	{ 
		std::cout << "Right button pressed" << std::endl;
		return 0;
	}

	case WM_RBUTTONUP:
	{ 
		std::cout << "Right button released" << std::endl;
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		std::cout << "Mouse wheel scrolled: " << delta << std::endl;
		return 0;
	}

	case WM_CLOSE:
	{   
		std::cout << "Window is closing" << std::endl;
	    DestroyWindow(hwnd); // This will trigger WM_DESTROYDestroyWindow(hwnd); // This will trigger WM_DESTROY
		return 0;
	} 

	case WM_DESTROY:
	{
		std::cout << "Window destroyed" << std::endl;
		PostQuitMessage(0);  //set the msg.message to WM_QUIT
		return 0;
	} 

	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


SharedPtr<WindowBase> WindowFactory::createWindow(const WindowCreateInfo& createInfo)
{
	//parse the platform,  assume win32 here
#ifdef _WIN32
	return CreateShared<Win32Window>(createInfo);
#else
	return nullptr; //or throw an exception
#endif
}

Win32Window::Win32Window(const WindowCreateInfo& createInfo)
{
	m_width = createInfo.width;
	m_height = createInfo.height;


	//get the instance handle:
	m_hInstance = GetModuleHandle(nullptr);

	//init window class:
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;  
	windowClass.hInstance = m_hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"TestWindowClass";
	if (!RegisterClassEx(&windowClass))
	{
		throw std::runtime_error("Failed to register window class");
	}


	//auto wideTitle = std::wstring(createInfo.title.begin(), createInfo.title.end()).c_str();
	std::wstring wideTitle = std::wstring(createInfo.title.begin(), createInfo.title.end());
	const wchar_t* titlePtr = wideTitle.c_str();

	//create the window:
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		titlePtr,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		createInfo.width,
		createInfo.height,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		m_hInstance,
		this); //pass "this" as the lpCreateParams

	if (!m_hwnd)
	{
		throw std::runtime_error("Failed to create window");
	}

	ShowWindow(m_hwnd, SW_SHOW);

	 
}

bool Win32Window::shouldClose()
{ 
	return m_msg.message == WM_QUIT;
	//return false;
}

void Win32Window::onUpdate()
{  
	this->pollEvents(); 
}

void Win32Window::SetCustomWindowText(const std::string& text) const
{
	std::wstring wideText = std::wstring(text.begin(), text.end());
	SetWindowText(m_hwnd, wideText.c_str());
}

void Win32Window::pollEvents()
{
	//while (m_msg.message != WM_QUIT) 
	if (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}
}


