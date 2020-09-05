#include "Lights.hlsli"


Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);

SamplerState gsamLinear : register(s0);

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return normalize(bumpedNormalW);
}

cbuffer cbPerFrame : register(b1)
{
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
    float gPad;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
    float2 gTextureScale;
    float2 gTextureDisplacement;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    float3 normalMapSample = gNormalMap.Sample(gsamLinear, (gTextureScale * pin.TexC) + gTextureDisplacement).rgb;
    
    float3 bumpedNormal = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
    
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// Start with a sum of zero. 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
    float4 A, D, S;

    ComputeDirectionalLight(gMaterial, gDirLight, bumpedNormal, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputePointLight(gMaterial, gPointLight, pin.PosW, bumpedNormal, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, bumpedNormal, toEyeW, A, D, S);
    ambient += max(A, 0);
    diffuse += max(D, 0);
    spec += max(S, 0);
	   
    float4 litColor = ambient + diffuse + spec;

	// Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a;

    float4 diffuseAlbedo = litColor * gDiffuseMap.Sample(gsamLinear, (gTextureScale * pin.TexC) + gTextureDisplacement);
    
    return diffuseAlbedo;
}