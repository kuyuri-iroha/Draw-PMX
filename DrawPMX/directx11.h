#pragma once

#include "api.h"
#include <memory>
#include "windows.h"


// Direct11Œn
class DirectX11
{
private:
	IDXGIFactory* pFactory;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pContext;
	IDXGISwapChain* pSwapChain;

	ID3D11BlendState* pBlendState;
	enum RSType
	{
		STANDARD,
		NOT_CULLING,
		WIREFRAME,
		RSType_SIZE
	};
	ID3D11RasterizerState* pRasterizerState[RSType_SIZE];
	ID3D11DepthStencilState* pDepthStencilState;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11DepthStencilView* pDepthStencilView;

public:
	HRESULT init(Windows& _window);
	void display();
	void end();

};