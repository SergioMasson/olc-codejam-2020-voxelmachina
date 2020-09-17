#include "NewLightModel.hlsli"
static const float PI = 3.141592;
static const float Epsilon = 0.00001;
static const uint NumLights = 3;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;


cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
    float2 gTextureScale;
    float2 gTextureDisplacement;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 Tex : TEXCOORD;
};

struct PixelShaderInput
{
    float4 pixelPosition : SV_POSITION;
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
};

// Vertex shader
PixelShaderInput main(VertexIn vin)
{
    PixelShaderInput vout;
    vout.position = mul(float4(vin.PosL, 1.0), gWorld).xyz;
    vout.texcoord = vin.Tex;

    //vout.NormalW = mul(vin.NormalL, ((float3x3) gWorldInvTranspose));
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3) gWorld);
    
    vout.pixelPosition = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
    return vout;
}