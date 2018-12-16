#include "Directx11.h"
#include "utility.h"


HRESULT DirectX11::init(Windows& _window)
{
	// Device & Device context ----------------
	D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL supportedFeatureLevels{};

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevels, 1, D3D11_SDK_VERSION, &pDevice, &supportedFeatureLevels, &pContext);
	if (FAILED(result))
	{
		return result;
	}

	// (MakeWindowAssociation()を正常動作させるための手順)
	// 参考: https://stackoverflow.com/questions/2353178/disable-alt-enter-in-a-direct3d-directx-application
	IDXGIDevice* pIDXGIDevice{};
	result = pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pIDXGIDevice);
	if (FAILED(result))
	{
		return result;
	}

	IDXGIAdapter* pAdapter{};
	result = pIDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);
	if (FAILED(result))
	{
		return result;
	}

	// IDXGIFactory ---------------------------
	result = pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);
	if (FAILED(result))
	{
		return result;
	}
	SafeRelease(pAdapter);
	SafeRelease(pIDXGIDevice);

	// Swap chain -----------------------------
	DXGI_SAMPLE_DESC msaaQuality{};
	for (int i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i <<= 1)
	{
		unsigned quality{};
		if (SUCCEEDED(pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, i, &quality)))
		{
			if (0 < quality)
			{
				msaaQuality.Count = i;
				msaaQuality.Quality = quality - 1;
			}
		}
	}
	DXGI_SWAP_CHAIN_DESC spDesc{};
	spDesc.BufferCount = 1;
	spDesc.BufferDesc.Width = Windows::WINDOW_WIDTH;
	spDesc.BufferDesc.Height = Windows::WINDOW_HEIGHT;
	spDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	spDesc.BufferDesc.RefreshRate.Numerator = 60;
	spDesc.BufferDesc.RefreshRate.Denominator = 1;
	spDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	spDesc.OutputWindow = _window.getWindowHandle();
	spDesc.SampleDesc = msaaQuality;
	spDesc.Windowed = true;
	spDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = pFactory->CreateSwapChain(pDevice, &spDesc, &pSwapChain);
	if (FAILED(result))
	{
		return result;
	}

	pFactory->MakeWindowAssociation(_window.getWindowHandle(), DXGI_MWA_NO_ALT_ENTER);

	// Blend state ----------------------------
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = pDevice->CreateBlendState(&blendDesc, &pBlendState);
	if (FAILED(result))
	{
		return result;
	}

	// Rasterizer -----------------------------
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE; //深度値クリッピング
	rasterizerDesc.ScissorEnable = FALSE; //シザー矩形クリッピング
	rasterizerDesc.MultisampleEnable = TRUE; //マルチサンプリング
	rasterizerDesc.AntialiasedLineEnable = TRUE; //線のマルチサンプリング

	// standard
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState[STANDARD]);
	if (FAILED(result))
	{
		return result;
	}
	// not culling
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState[NOT_CULLING]);
	if (FAILED(result))
	{
		return result;
	}
	// wireframe
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState[WIREFRAME]);
	if (FAILED(result))
	{
		return result;
	}
	// not depth clip
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = FALSE;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState[NOT_DEPTH_CLIP]);
	if (FAILED(result))
	{
		return result;
	}

	// Depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = 0;
	dsDesc.StencilWriteMask = 0;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = pDevice->CreateDepthStencilState(&dsDesc, &pDepthStencilState);
	if (FAILED(result))
	{
		return result;
	}

	// Back buffer ----------------------------
	ID3D11Texture2D* pBackBuffer{};
	result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(result))
	{
		return result;
	}

	D3D11_TEXTURE2D_DESC backBufferDesc;
	pBackBuffer->GetDesc(&backBufferDesc);

	result = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	if (FAILED(result))
	{
		SafeRelease(pBackBuffer);
		return result;
	}

	result = pDevice->CreateShaderResourceView(pBackBuffer, NULL, &pScreenSRV);
	if (FAILED(result))
	{
		SafeRelease(pBackBuffer);
		return result;
	}
	SafeRelease(pBackBuffer);

	D3D11_TEXTURE2D_DESC depthStencilDesc = backBufferDesc;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc = backBufferDesc.SampleDesc;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = NULL;
	depthStencilDesc.MiscFlags = NULL;

	ID3D11Texture2D* pDepthStencil{};
	result = pDevice->CreateTexture2D(&depthStencilDesc, NULL, &pDepthStencil);
	if (FAILED(result))
	{
		SafeRelease(pDepthStencil);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = pDevice->CreateDepthStencilView(pDepthStencil, &depthStencilViewDesc, &pDepthStencilView);
	SafeRelease(pDepthStencil);
	if (FAILED(result))
	{
		return false;
	}

	// レンダリングパイプラインの設定 ---------------------------
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->RSSetState(pRasterizerState[STANDARD]);
	pContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);
	constexpr FLOAT blendFactor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
	pContext->OMSetDepthStencilState(pDepthStencilState, 0);

	return result;
}


void DirectX11::display()
{
	pSwapChain->Present(1, 0);
	constexpr float CLEAR_COLOR[]{1.0f, 1.0f, 1.0f, 1.0f};
	pContext->ClearRenderTargetView(pRenderTargetView, CLEAR_COLOR);
	pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void DirectX11::end()
{
	if(pContext) {pContext->ClearState();}
	if(pSwapChain) {pSwapChain->SetFullscreenState(FALSE, NULL);}
	SafeRelease(pDepthStencilState);
	for (int i = 0; i<RSMode_SIZE; ++i)
	{
		SafeRelease(pRasterizerState[i]);
	}
	SafeRelease(pScreenSRV);
	SafeRelease(pBlendState);
	SafeRelease(pDepthStencilView);
	SafeRelease(pRenderTargetView);
	SafeRelease(pSwapChain);
	SafeRelease(pContext);
	SafeRelease(pFactory);
	SafeRelease(pDevice);
}