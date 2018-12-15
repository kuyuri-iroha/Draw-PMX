#pragma once

#include "api.h"
#include <memory>
#include <array>
#include "Windows.h"


// Direct11Œn
class DirectX11
{
public:
	enum RSMode
	{
		STANDARD,
		NOT_CULLING,
		WIREFRAME,
		RSMode_SIZE
	};

private:
	IDXGIFactory* pFactory;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pContext;
	IDXGISwapChain* pSwapChain;

	ID3D11BlendState* pBlendState;
	std::array<ID3D11RasterizerState*, RSMode_SIZE> pRasterizerState;
	ID3D11DepthStencilState* pDepthStencilState;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11DepthStencilView* pDepthStencilView;

public:
	inline ID3D11DeviceContext* const getContext() {return pContext;}
	inline ID3D11RasterizerState* const getRasterizerState(RSMode _mode) {return pRasterizerState[_mode];}

	HRESULT init(Windows& _window);
	void display();
	void end();

};