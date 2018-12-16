#include "EdgeEffector.h"
#include "utility.h"



HRESULT EdgeEffector::init(ID3D11Device* const _pDevice)
{
	// 頂点データ -------------------------------
	// 参考: https://stackoverflow.com/questions/20412807/directx-11-make-a-square
	std::array<XMFLOAT3, VERTEX_COUNT> vertices{};
	vertices[0] = XMFLOAT3{-1.0f, -1.0f, 0.0f};  // Bottom left.
	vertices[1] = XMFLOAT3{-1.0f, 1.0f, 0.0f};  // Top left.
	vertices[2] = XMFLOAT3{1.0f, 1.0f, 0.0f};  // top right.
	vertices[3] = XMFLOAT3{-1.0f, -1.0f, 0.0f};  // Bottom left.
	vertices[4] = XMFLOAT3{1.0f, 1.0f, 0.0f};  // top right.
	vertices[5] = XMFLOAT3{1.0f, -1.0f, 0.0f};  // Bottom right.
	std::array<XMFLOAT2, VERTEX_COUNT> uv{};
	uv[0] = XMFLOAT2{0.0f, 1.0f};
	uv[1] = XMFLOAT2{0.0f, 0.0f};
	uv[2] = XMFLOAT2{1.0f, 0.0f};
	uv[3] = XMFLOAT2{0.0f, 1.0f};
	uv[4] = XMFLOAT2{1.0f, 0.0f};
	uv[5] = XMFLOAT2{1.0f, 1.0f};
	std::array<unsigned, VERTEX_COUNT> indices{};
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top left.
	indices[2] = 2;  // top right.
	indices[3] = 3;  // Bottom left.
	indices[4] = 4;  // Top left.
	indices[5] = 5;  // top right.
	indexCount = VERTEX_COUNT;


	// 頂点バッファ -----------------------------

	// 頂点座標
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * VERTEX_COUNT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = &vertices[0];
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	HRESULT result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pVertexBuffers[POSITION]);
	if (FAILED(result))
	{
		return false;
	}

	// UV座標
	bufferDesc.ByteWidth = sizeof(XMFLOAT2) * VERTEX_COUNT;
	subresourceData.pSysMem = &uv[0];
	result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pVertexBuffers[UV]);
	if (FAILED(result))
	{
		return false;
	}

	// インデックスバッファ ---------------------
	bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(unsigned) * VERTEX_COUNT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	subresourceData = {};
	subresourceData.pSysMem = &indices[0];
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 入力エレメント ---------------------------
	std::array<D3D11_INPUT_ELEMENT_DESC, VertexBuffer_SIZE> ieDescs;

	ieDescs[POSITION].SemanticName = "POSITION";
	ieDescs[POSITION].SemanticIndex = 0;
	ieDescs[POSITION].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ieDescs[POSITION].InputSlot = POSITION;
	ieDescs[POSITION].AlignedByteOffset = 0;
	ieDescs[POSITION].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ieDescs[POSITION].InstanceDataStepRate = 0;

	ieDescs[UV].SemanticName = "TEXCOORD";
	ieDescs[UV].SemanticIndex = 0;
	ieDescs[UV].Format = DXGI_FORMAT_R32G32_FLOAT;
	ieDescs[UV].InputSlot = UV;
	ieDescs[UV].AlignedByteOffset = 0;
	ieDescs[UV].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ieDescs[UV].InstanceDataStepRate = 0;

	// 頂点シェーダー ---------------------------
	ID3DBlob* pByteCode{};

	result = compileShader(L"shaders\\edge.hlsl", "vsMain", true, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 入力レイアウト
	result = _pDevice->CreateInputLayout(&ieDescs[0], ieDescs.size(), pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), &pInputLayout);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 頂点シェーダー
	result = _pDevice->CreateVertexShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &pVertexShader);
	SafeRelease(pByteCode);
	if (FAILED(result))
	{
		return result;
	}

	// ピクセルシェーダー -----------------------
	result = compileShader(L"shaders\\edge.hlsl", "psMain", false, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	result = _pDevice->CreatePixelShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &pPixelShader);
	SafeRelease(pByteCode);
	if (FAILED(result))
	{
		return result;
	}

	// 定数バッファ -----------------------------
	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	cbDesc.ByteWidth = 16;

	result = _pDevice->CreateBuffer(&cbDesc, NULL, &pConstantBuffer);
	if (FAILED(result))
	{
		return result;
	}

	// サンプラーステート -----------------------
	D3D11_SAMPLER_DESC spDesc;
	spDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	spDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	spDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	spDesc.MipLODBias = 0.0f;
	spDesc.MaxAnisotropy = 0;
	spDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	spDesc.BorderColor[0] = 0.0f;
	spDesc.BorderColor[1] = 0.0f;
	spDesc.BorderColor[2] = 0.0f;
	spDesc.BorderColor[3] = 0.0f;
	spDesc.MaxLOD = FLT_MAX;
	spDesc.MinLOD = -FLT_MAX;
	spDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	result = _pDevice->CreateSamplerState(&spDesc, &pSamplerState);

	return result;
}


void EdgeEffector::draw(DirectX11& _directX11, const void* const _pConstantBufferData)
{
	ID3D11DeviceContext* const & pContext = _directX11.getContext();

	// シェーダーのセット
	pContext->IASetInputLayout(pInputLayout);
	pContext->VSSetShader(pVertexShader, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pContext->PSSetShader(pPixelShader, NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	pContext->PSSetSamplers(0, 1, &pSamplerState);
	pContext->PSSetShaderResources(0, 1, &_directX11.pScreenSRV);

	// 定数バッファの更新
	pContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	D3D11_BUFFER_DESC cbDesc{};
	pConstantBuffer->GetDesc(&cbDesc);
	CopyMemory(resource.pData, _pConstantBufferData, cbDesc.ByteWidth);
	pContext->Unmap(pConstantBuffer, 0);


	pContext->RSSetState(_directX11.getRasterizerState(DirectX11::NOT_DEPTH_CLIP));

	constexpr std::array<unsigned, VertexBuffer_SIZE> stride{ sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	constexpr std::array<unsigned, VertexBuffer_SIZE> offset{ 0, 0};
	pContext->IASetVertexBuffers(0, VertexBuffer_SIZE, &pVertexBuffers[0], &stride[0], &offset[0]);

	pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pContext->DrawIndexed(VERTEX_COUNT, 0, 0);
}


void EdgeEffector::end()
{
	SafeRelease(pTexture);
	SafeRelease(pConstantBuffer);
	SafeRelease(pSamplerState);
	SafeRelease(pPixelShader);
	SafeRelease(pVertexShader);
	SafeRelease(pInputLayout);
	SafeRelease(pIndexBuffer);
	for (auto* pVertexBuffer : pVertexBuffers)
	{
		SafeRelease(pVertexBuffer);
	}
}