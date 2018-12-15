#include "api.h"
#include "windows.h"
#include "directx11.h"

Windows window = {};
DirectX11 directX11 = {};


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// COM系の初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// 算術ライブラリが使用できるかの検証
	if (XMVerifyCPUSupport() != TRUE)
	{
		return 1;
	}

	// 初期化
	window.init();
	directX11.init(window);

	// メインループ
	while (window.processMessage())
	{
		directX11.display();
	}

	//終了
	directX11.end();

	CoUninitialize();

	return 0;
}