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
	for (int i = 0; i < vertices.size(); i++)
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
	for (int i = 0; i < uv.size(); i++)
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
	for (int i = 0; i < vertices.size(); i++)
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
	mIndexCount = data.surfaces.size();

	// メッシュ -------------------------------
	meshes.resize(data.materials.size());
	D3DX11_IMAGE_LOAD_INFO loadInfo{};
	D3DX11_IMAGE_INFO imageInfo{};
	for (int i = 0; i < meshes.size(); ++i)
	{
		if (data.materials[i].colorMapTextureIndex != PMXModelData::NO_DATA_FLAG)
		{
			// テクスチャ
			result = D3DX11GetImageInfoFromFileW(data.texturePaths[data.materials[i].colorMapTextureIndex].c_str(), NULL, &imageInfo, NULL);
			if (FAILED(result))
			{
				return;
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
		}

		meshes[i].vertexNum = data.materials[i].vertexNum;
		meshes[i].diffuseColor = data.materials[i].diffuse;
		meshes[i].specularColor = data.materials[i].specular;
		meshes[i].specularity = data.materials[i].specularity;
		meshes[i].ambientColor = data.materials[i].ambient;
	}
}


void Model::draw(DirectX11& _directX11)
{
	ID3D11DeviceContext* const pContext = _directX11.getContext();

	pContext->RSSetState(_directX11.getRasterizerState(DirectX11::NOT_CULLING));

	constexpr std::array<unsigned, VertexBuffer_SIZE> stride{sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT3)};
	constexpr std::array<unsigned, VertexBuffer_SIZE> offset{0, 0, 0};
	pContext->IASetVertexBuffers(0, VertexBuffer_SIZE, &pVertexBuffers[0], &stride[0], &offset[0]);

	pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	unsigned drewVertexNum{};
	for (auto mesh : meshes)
	{
		pContext->PSSetShaderResources(0, 1, &mesh.pTexture);
		pContext->DrawIndexed(mesh.vertexNum, drewVertexNum, 0);

		drewVertexNum += mesh.vertexNum;
	}
}


void Model::end()
{
	for (auto pVertexBuffer : pVertexBuffers)
	{
		SafeRelease(pVertexBuffers);
	}
	SafeRelease(pIndexBuffer);
	for (auto mesh : meshes)
	{
		SafeRelease(mesh.pTexture);
	}
}