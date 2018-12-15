#pragma once

#pragma warning(disable:4005) //DirectXŒnƒ}ƒNƒ‚ÌÄ’è‹`Œx‚ğ”ñ•\¦
#pragma warning(disable:4838) //XNAMath‚Ìk¬•ÏŠ·Œx‚ğ”ñ•\¦

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