#include "NewLightModel.hlsli"


Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gEmissionMap : register(t2);

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
    float3 bumpedNormalW = mul(normalize(normalT), TBN);

    return bumpedNormalW;
}

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
    float2 gTextureScale;
    float2 gTextureDisplacement;
};

cbuffer cbPerFrame : register(b1)
{
    float3 gEyePosW;
    int NumberOfLights;
    Light Lights[MAX_LIGHTS];
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

    float4 albedoMapColor = gDiffuseMap.Sample(gsamLinear, (gTextureScale * pin.TexC) + gTextureDisplacement);
    float4 objectColor = gMaterial.Color * albedoMapColor;
    
    float4 pixelColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    pixelColor += gEmissionMap.Sample(gsamLinear, pin.TexC);
    pixelColor += gMaterial.Emission;
    
    [unroll] 
    for (int i = 0; (i < NumberOfLights) && (i < MAX_LIGHTS); i++)
    {  
        if (Lights[i].LightType == DIR_LIGHT)
            pixelColor += objectColor * ComputeDirectionalLight(gMaterial, Lights[i], bumpedNormal, toEyeW);
        
        if (Lights[i].LightType == SPOT_LIGHT)
            pixelColor += objectColor * ComputePointLight(gMaterial, Lights[i], pin.PosW, bumpedNormal, toEyeW);
        
        if (Lights[i].LightType == POINT_LIGHT)
            pixelColor += objectColor * ComputeSpotLight(gMaterial, Lights[i], pin.PosW, bumpedNormal, toEyeW);
    }
    
    return pixelColor;
}