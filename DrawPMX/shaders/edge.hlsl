
// 定数バッファ
cbuffer cb : register(b0)
{
	float2 resolution;
    float2 dammy;
}

// 頂点シェーダー --------------------------------------
struct VSInput
{
	float3 pos : POSITION; //頂点座標
	float2 uv : TEXCOORD0; //テクスチャ座標
};

struct VSOutput
{
	float4 pos : SV_POSITION; //頂点座標
	float2 uv : TEXCOORD0; //テクスチャ座標
};


VSOutput vsMain(VSInput input)
{
    VSOutput output;

    // 頂点座標
    output.pos = float4(input.pos, 1.0);

    // テクスチャ座標
    output.uv = input.uv;

    return output;
}


// ピクセルシェーダー ----------------------------------
struct PSInput
{
	float4 pos : SV_POSITION; //頂点座標
	float2 uv : TEXCOORD0; //テクスチャ座標
};

// テクスチャ
Texture2D bPostTexture : register(t0);
SamplerState postSampler : register(s0);

float4 psMain(PSInput input) : SV_TARGET
{
    float4 color;
    float4 textureColor = bPostTexture.Sample(postSampler, input.uv);

    return textureColor;
}