#include "Windows.h"
#include <string>


bool inAction = {};


LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		inAction = false;
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


bool Windows::init()
{
	inAction = false;
	const std::wstring className{L"draw_pmx_2018_12_14"};
	const std::wstring windowName{L"DrawPMX"};
	moduleHandle = NULL;
	windowHandle = NULL;

	moduleHandle = GetModuleHandle(NULL);

	// Windowクラスの登録
	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(wcx);
	wcx.style = (CS_HREDRAW | CS_VREDRAW);
	wcx.lpfnWndProc = wndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = moduleHandle;
	wcx.hIcon = static_cast<HICON>(LoadImage(moduleHandle, IDI_APPLICATION, IMAGE_ICON, 0, 0, (LR_DEFAULTSIZE | LR_SHARED)));
	wcx.hCursor = static_cast<HCURSOR>(LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, (LR_DEFAULTSIZE | LR_SHARED)));
	wcx.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = className.c_str();
	wcx.hIconSm = static_cast<HICON>(LoadImage(moduleHandle, IDI_APPLICATION, IMAGE_ICON, 0, 0, (LR_DEFAULTSIZE | LR_SHARED)));

	if (RegisterClassEx(&wcx) == 0)
	{
		return false;
	}

	// Windowの作成
	windowHandle = CreateWindowW(className.c_str(), windowName.c_str(), (WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION),
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, moduleHandle, NULL);

	if (windowHandle == NULL)
	{
		return false;
	}

	// Windowサイズの調整
	RECT client;
	RECT window;
	GetClientRect(windowHandle, &client);
	GetWindowRect(windowHandle, &window);
	MoveWindow(windowHandle, window.left, window.top,
		WINDOW_WIDTH + (WINDOW_WIDTH - client.right), WINDOW_HEIGHT + (WINDOW_HEIGHT - client.bottom), TRUE);

	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);

	inAction = true;
	return true;
}


bool Windows::processMessage()
{
	MSG msg = {};

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return inAction;
}

