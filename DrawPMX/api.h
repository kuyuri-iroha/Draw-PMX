#pragma once

#pragma warning(disable:4005) //DirectX系マクロの再定義警告を非表示
#pragma warning(disable:4838) //XNAMathの縮小変換警告を非表示

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment( lib, "dxerr.lib")
#pragma comment( lib, "dxgi.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#endif