#pragma once

#include "api.h"


template<class COMPointer>
void SafeRelease(COMPointer& _p)
{
	if (_p)
	{
		_p->Release();
		_p = nullptr;
	}
}

// 誤差値
constexpr float EPSILON = 1e-8f;


// シェーダーのコンパイル
static inline HRESULT compileShader(const std::wstring& _filePath, const std::string& _entryPoint, bool _isVertexShader, ID3DBlob*& pByteCode)
{
	HRESULT result{};
	std::string shaderType{ _isVertexShader ? "vs_5_0" : "ps_5_0" };

	D3DX11CompileFromFile(_filePath.c_str(), NULL, NULL, _entryPoint.c_str(), shaderType.c_str(),
		(D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR), NULL, NULL, &pByteCode, NULL, &result);

	return result;
}