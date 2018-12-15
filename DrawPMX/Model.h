#pragma once

#include "api.h"
#include <array>
#include "Directx11.h"


// ÉÅÉbÉVÉÖ
struct Mesh
{
	unsigned vertexNum;
	ID3D11ShaderResourceView* pTexture;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 specularColor;
	float specularity;
	XMFLOAT3 ambientColor;
};


// ÉÇÉfÉãï`âÊ
class Model
{
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
	unsigned mIndexCount;
	std::vector<Mesh> meshes;

public:
	bool init(const std::wstring& _filePath, ID3D11Device* const _pDevice);
	void draw(DirectX11& _directX11);
	void end();

};

