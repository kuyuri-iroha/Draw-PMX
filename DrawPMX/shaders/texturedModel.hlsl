
// 定数バッファ
cbuffer cb : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;

    float3 specularColor;
    float specularity;
    float3 lightDir;
    float dammy;
    float3 eyeDir;
    float dammy2;
}


// 頂点シェーダー --------------------------------------
struct VSInput
{
	float3 pos : POSITION; //頂点座標(モデル座標系)
	float2 uv : TEXCOORD0; //テクスチャ座標
	float3 norm : NORMAL; //法線ベクトル
};

struct VSOutput
{
	float4 pos : SV_POSITION; //頂点座標(透視変換座標)
	float2 uv : TEXCOORD0; //テクスチャ座標
	float3 norm : TEXCOORD1; //法線ベクトル
};


VSOutput vsMain(VSInput input)
{
    VSOutput output;

    // 頂点座標変換
    output.pos = mul(mul(mul(float4(input.pos, 1.0f), world), view), projection);

    // テクスチャ座標
    output.uv = input.uv;

    // 法線ベクトル
    output.norm = mul(mul(input.norm, (float3x3)world), (float3x3)view);

    return output;
}


// ピクセルシェーダー ----------------------------------
struct PSInput
{
	float4 pos : SV_POSITION; //頂点座標(透視変換座標)
	float2 uv : TEXCOORD0; //テクスチャ座標
	float3 norm : TEXCOORD1; //法線ベクトル
};

// テクスチャ
Texture2D modelTexture : register(t0);
SamplerState modelSampler : register(s0);

float4 psMain(PSInput input) : SV_TARGET
{
    float4 color;
    float4 textureColor = modelTexture.Sample(modelSampler, input.uv);

    float normalizedNormDotLight = (dot(input.norm, lightDir) + 1.0) / 2.0;
    float brightness = saturate(normalizedNormDotLight + step(specularity, normalizedNormDotLight));

    color.rgb = textureColor.rgb * brightness;
    color.a = textureColor.a;

    if(brightness <= 0.6)
    {
        color.rgb = color.rgb * 0.2;
    }
    else if(0.95 <= brightness)
    {
        color.rgb = color.rgb * 1.5;
    }

    if(dot(input.norm, eyeDir) <= 0.3)
    {
        color.rgb = float3(0.0, 0.0, 0.0);
    }

    return color;
}