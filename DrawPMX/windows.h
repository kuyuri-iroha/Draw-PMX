#pragma once

#include "api.h"

// WindowsAPIŒn
class Windows
{
public:
	static constexpr unsigned WINDOW_WIDTH = 500;
	static constexpr unsigned WINDOW_HEIGHT = 500;

	bool init();
	bool processMessage();

	inline HWND getWindowHandle() {return windowHandle;}

private:
	HINSTANCE moduleHandle;
	HWND windowHandle;
};