#pragma once

#include "api.h"
#include <array>
#include "Directx11.h"


// edgeシェーダーの定数バッファデータ型
struct EdgeConstantBuffer
{
	XMVECTOR resolution;
};


// Edgeエフェクト
class EdgeEffector
{
public:
	static constexpr int VERTEX_COUNT = 6;
	HRESULT init(ID3D11Device* const _pDevice);
	void draw(DirectX11& _directX11, const void* const _pConstantBufferData);
	void end();

private:
	ID3D11InputLayout * pInputLayout;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11Buffer* pConstantBuffer;
	D3D11_MAPPED_SUBRESOURCE resource;
	ID3D11SamplerState* pSamplerState;
	ID3D11ShaderResourceView* pTexture;

	enum VertexBuffer
	{
		POSITION,
		UV,
		VertexBuffer_SIZE,
	};

	std::array<ID3D11Buffer*, VertexBuffer_SIZE> pVertexBuffers;
	ID3D11Buffer* pIndexBuffer;
	unsigned indexCount;

};

