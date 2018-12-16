#pragma once

#include "api.h"
#include <vector>
#include "Directx11.h"


// texturedModelシェーダーの定数バッファデータ型
struct TexturedModelConstantBufferData
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;

	XMVECTOR specular;
	XMVECTOR lightDir;
};

// notTexturedModelシェーダーの定数バッファデータ型
struct NotTexturedModelConstantBufferData
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;

	XMVECTOR diffuseColor;
	XMVECTOR ambientColor;
};


// メッシュ
struct Mesh
{
	unsigned vertexNum;
	ID3D11ShaderResourceView* pTexture;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 specularColor;
	float specularity;
	XMFLOAT3 ambientColor;

	// Shader
	ID3D11InputLayout* pInputLayout;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11Buffer* pConstantBuffer;
	D3D11_MAPPED_SUBRESOURCE resource;
	ID3D11SamplerState* pSamplerState;
};


// モデル描画
class Model
{
public:
	inline size_t getMeshesSize() {return meshes.size();}
	inline bool meshHasTexture(int _meshIndex) {return meshes[_meshIndex].pTexture != nullptr;}
	bool init(const std::wstring& _filePath, ID3D11Device* const _pDevice);
	void drawMesh(DirectX11& _directX11, int _meshIndex, const void* const _pConstantBufferData);
	void end();

	std::vector<Mesh> meshes;

private:
	enum VertexBuffer
	{
		POSITION,
		UV,
		NORMAL,
		VertexBuffer_SIZE,
	};

	std::array<ID3D11Buffer*, VertexBuffer_SIZE> pVertexBuffers;
	ID3D11Buffer* pIndexBuffer;
	unsigned indexCount;

	HRESULT createTexturedShader(ID3D11Device* const _pDevice, Mesh& mesh);
	HRESULT createNotTexturedShader(ID3D11Device* const _pDevice, Mesh& mesh);
};

