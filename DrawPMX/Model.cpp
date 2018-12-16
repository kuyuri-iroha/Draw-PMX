#include "Model.h"
#include "pmx_loader.h"
#include "utility.h"


bool Model::init(const std::wstring& _filePath, ID3D11Device* const _pDevice)
{
	PMXModelData data{};
	if (!loadPMX(data, _filePath))
	{
		return false;
	}

	// 頂点バッファ ---------------------------
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * data.vertices.size();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	std::vector<XMFLOAT3> vertices;
	vertices.resize(data.vertices.size());
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		vertices[i] = data.vertices[i].position;
	}
	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = &vertices[0];
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	HRESULT result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pVertexBuffers[POSITION]);
	if (FAILED(result))
	{
		return false;
	}

	// UV座標 ---------------------------------
	bufferDesc.ByteWidth = sizeof(XMFLOAT2) * data.vertices.size();
	std::vector<XMFLOAT2> uv;
	uv.resize(data.vertices.size());
	for (unsigned i = 0; i < uv.size(); i++)
	{
		uv[i] = data.vertices[i].uv;
	}
	subresourceData.pSysMem = &uv[0];
	result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pVertexBuffers[UV]);
	if (FAILED(result))
	{
		return false;
	}

	// 法線バッファ ---------------------------
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * data.vertices.size();
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		vertices[i] = data.vertices[i].normal;
	}
	subresourceData.pSysMem = &vertices[0];
	result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pVertexBuffers[NORMAL]);
	if (FAILED(result))
	{
		return false;
	}

	// インデックスバッファ -------------------
	bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(PMXModelData::Surface) * data.surfaces.size();
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	subresourceData = {};
	subresourceData.pSysMem = &data.surfaces[0];
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	result = _pDevice->CreateBuffer(&bufferDesc, &subresourceData, &pIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}
	indexCount = data.surfaces.size();

	// メッシュ -------------------------------
	meshes.resize(data.materials.size());
	D3DX11_IMAGE_LOAD_INFO loadInfo{};
	D3DX11_IMAGE_INFO imageInfo{};
	for (unsigned i = 0; i < meshes.size(); ++i)
	{
		if (data.materials[i].colorMapTextureIndex != 255)
		{
			// テクスチャ
			result = D3DX11GetImageInfoFromFileW(data.texturePaths[data.materials[i].colorMapTextureIndex].c_str(), NULL, &imageInfo, NULL);
			if (FAILED(result))
			{
				return false;
			}
			loadInfo.Width = imageInfo.Width;
			loadInfo.Height = imageInfo.Height;
			loadInfo.Depth = imageInfo.Depth;
			loadInfo.FirstMipLevel = 0;
			loadInfo.MipLevels = imageInfo.MipLevels;
			loadInfo.Usage = D3D11_USAGE_DEFAULT;
			loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			loadInfo.CpuAccessFlags = NULL;
			loadInfo.MiscFlags = NULL;
			loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			loadInfo.Filter = D3DX11_FILTER_LINEAR | D3DX11_FILTER_SRGB_IN;
			loadInfo.MipFilter = D3DX11_FILTER_LINEAR;
			loadInfo.pSrcInfo = &imageInfo;

			result = D3DX11CreateShaderResourceViewFromFileW(_pDevice, data.texturePaths[data.materials[i].colorMapTextureIndex].c_str(), &loadInfo, NULL, &meshes[i].pTexture, nullptr);
			if (FAILED(result))
			{
				return false;
			}
			
			// テクスチャシェーダー
			createTexturedShader(_pDevice, meshes[i]);
		}
		else
		{
			// 単色シェーダー
			createNotTexturedShader(_pDevice, meshes[i]);
		}

		meshes[i].vertexNum = data.materials[i].vertexNum;
		meshes[i].diffuseColor = data.materials[i].diffuse;
		meshes[i].specularColor = data.materials[i].specular;
		meshes[i].specularity = data.materials[i].specularity;
		meshes[i].ambientColor = data.materials[i].ambient;
	}

	return true;
}


void Model::drawMesh(DirectX11& _directX11, int _meshIndex, const void* const _pConstantBufferData)
{
	ID3D11DeviceContext* const & pContext = _directX11.getContext();

	// シェーダーのセット
	pContext->IASetInputLayout(meshes[_meshIndex].pInputLayout);
	pContext->VSSetShader(meshes[_meshIndex].pVertexShader, NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, &meshes[_meshIndex].pConstantBuffer);
	pContext->PSSetShader(meshes[_meshIndex].pPixelShader, NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, &meshes[_meshIndex].pConstantBuffer);
	pContext->PSSetSamplers(0, 1, &meshes[_meshIndex].pSamplerState);
	pContext->PSSetShaderResources(0, 1, &meshes[_meshIndex].pTexture);

	// 定数バッファの更新
	pContext->Map(meshes[_meshIndex].pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &meshes[_meshIndex].resource);
	D3D11_BUFFER_DESC cbDesc{};
	meshes[_meshIndex].pConstantBuffer->GetDesc(&cbDesc);
	CopyMemory(meshes[_meshIndex].resource.pData, _pConstantBufferData, cbDesc.ByteWidth);
	pContext->Unmap(meshes[_meshIndex].pConstantBuffer, 0);


	pContext->RSSetState(_directX11.getRasterizerState(DirectX11::NOT_CULLING));

	constexpr std::array<unsigned, VertexBuffer_SIZE> stride{sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT3)};
	constexpr std::array<unsigned, VertexBuffer_SIZE> offset{0, 0, 0};
	pContext->IASetVertexBuffers(0, VertexBuffer_SIZE, &pVertexBuffers[0], &stride[0], &offset[0]);

	pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	int firstVertexIndex{};
	for (int i = 0; i < _meshIndex; i++)
	{
		firstVertexIndex += meshes[i].vertexNum;
	}

	pContext->DrawIndexed(meshes[_meshIndex].vertexNum, firstVertexIndex, 0);
}


void Model::end()
{
	for (auto mesh : meshes)
	{
		SafeRelease(mesh.pTexture);
		SafeRelease(mesh.pConstantBuffer);
		SafeRelease(mesh.pSamplerState);
		SafeRelease(mesh.pPixelShader);
		SafeRelease(mesh.pVertexShader);
		SafeRelease(mesh.pInputLayout);
	}
	SafeRelease(pIndexBuffer);
	for (auto pVertexBuffer : pVertexBuffers)
	{
		SafeRelease(pVertexBuffer);
	}
}


HRESULT Model::createTexturedShader(ID3D11Device* const _pDevice, Mesh& mesh)
{
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

	ieDescs[NORMAL].SemanticName = "NORMAL";
	ieDescs[NORMAL].SemanticIndex = 0;
	ieDescs[NORMAL].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ieDescs[NORMAL].InputSlot = NORMAL;
	ieDescs[NORMAL].AlignedByteOffset = 0;
	ieDescs[NORMAL].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ieDescs[NORMAL].InstanceDataStepRate = 0;

	// 頂点シェーダー ---------------------------
	ID3DBlob* pByteCode{};

	HRESULT result = compileShader(L"shaders\\texturedModel.hlsl", "vsMain", true, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 入力レイアウト
	result = _pDevice->CreateInputLayout(&ieDescs[0], ieDescs.size(), pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), &mesh.pInputLayout);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 頂点シェーダー
	result = _pDevice->CreateVertexShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &mesh.pVertexShader);
	SafeRelease(pByteCode);
	if (FAILED(result))
	{
		return result;
	}

	// ピクセルシェーダー -----------------------
	result = compileShader(L"shaders\\texturedModel.hlsl", "psMain", false, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	result = _pDevice->CreatePixelShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &mesh.pPixelShader);
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
	cbDesc.ByteWidth = 192;

	result = _pDevice->CreateBuffer(&cbDesc, NULL, &mesh.pConstantBuffer);
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

	result = _pDevice->CreateSamplerState(&spDesc, &mesh.pSamplerState);

	return result;
}


HRESULT Model::createNotTexturedShader(ID3D11Device* const _pDevice, Mesh& mesh)
{
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

	ieDescs[NORMAL].SemanticName = "NORMAL";
	ieDescs[NORMAL].SemanticIndex = 0;
	ieDescs[NORMAL].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ieDescs[NORMAL].InputSlot = NORMAL;
	ieDescs[NORMAL].AlignedByteOffset = 0;
	ieDescs[NORMAL].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ieDescs[NORMAL].InstanceDataStepRate = 0;

	// 頂点シェーダー ---------------------------
	ID3DBlob* pByteCode{};

	HRESULT result = compileShader(L"shaders\\notTexturedModel.hlsl", "vsMain", true, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 入力レイアウト
	result = _pDevice->CreateInputLayout(&ieDescs[0], ieDescs.size(), pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), &mesh.pInputLayout);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	// 頂点シェーダー
	result = _pDevice->CreateVertexShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &mesh.pVertexShader);
	SafeRelease(pByteCode);
	if (FAILED(result))
	{
		return result;
	}

	// ピクセルシェーダー -----------------------
	result = compileShader(L"shaders\\notTexturedModel.hlsl", "psMain", false, pByteCode);
	if (FAILED(result))
	{
		SafeRelease(pByteCode);
		return result;
	}

	result = _pDevice->CreatePixelShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), NULL, &mesh.pPixelShader);
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
	cbDesc.ByteWidth = 224;

	result = _pDevice->CreateBuffer(&cbDesc, NULL, &mesh.pConstantBuffer);
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

	result = _pDevice->CreateSamplerState(&spDesc, &mesh.pSamplerState);

	return result;
}