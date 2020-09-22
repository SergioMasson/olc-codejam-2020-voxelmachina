#include "NewLightModel.hlsli"

static const float PI = 3.141592;
static const float Epsilon = 0.00001;

static const uint NumLights = 3;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

struct Material2
{
    float4 Color;
    
    float4 Emission;
    
    float Metalness;
    float Roughness; // w = SpecPower
    float2 Pad;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material2 gMaterial;
    float2 gTextureScale;
    float2 gTextureDisplacement;
};

cbuffer cbPerFrame : register(b1)
{
    float3 gEyePosW;
    int NumberOfLights;
    Light Lights[MAX_LIGHTS];
};


struct PixelShaderInput
{
    float4 pixelPosition : SV_POSITION;
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
};

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


// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Pixel shader
float4 main(PixelShaderInput pin) : SV_Target
{
	// Sample input textures to get shading model params.
    float3 albedo = gMaterial.Color.rgb * gDiffuseMap.Sample(gsamLinear, (pin.texcoord * gTextureScale) + gTextureDisplacement).rgb;
    
    albedo = pow(albedo, 2.2f);
    
    float metalness = gMaterial.Metalness;
    float roughness = gMaterial.Roughness;

	// Outgoing light direction (vector from world-space fragment position to the "eye").
    float3 Lo = normalize(gEyePosW - pin.position);

	// Get current fragment's normal and transform to world space.
    float3 N = NormalSampleToWorldSpace(gNormalMap.Sample(gsamLinear, (pin.texcoord * gTextureScale) + gTextureDisplacement).rgb, pin.NormalW, pin.TangentW);
	
	// Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
    float3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
    float3 F0 = lerp(Fdielectric, albedo, metalness);

	// Direct lighting calculation for analytical lights.
    float3 directLighting = 0.0;
    
    for (uint i = 0; i < NumberOfLights; ++i)
    {
        float3 Lradiance = Lights[i].Color;
        float3 Li = -Lights[i].Direction;
        float att = 1;
		// Half-vector between Li and Lo.
        
        if (Lights[i].LightType == DIR_LIGHT)
        {
            Li = -Lights[i].Direction;
            Lradiance = Lights[i].Color * Lights[i].Intensity;
        }
        if (Lights[i].LightType == SPOT_LIGHT)
        {
            Li = Lights[i].Position - pin.position;
            // The distance from surface to light.
            float d = length(Li);
            att = CalcAttenuation(d, 0, Lights[i].Range);
        }
        if (Lights[i].LightType == POINT_LIGHT)
        {
            Li = Lights[i].Position - pin.position;
            // The distance from surface to light.
            float d = length(Li);
            att = CalcAttenuation(d, 0, Lights[i].Range);
            
            // Normalize the light vector.
            Li /= d;
        }
       
        float3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(N, Li));
        float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
        float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
        
		// Calculate normal distribution for specular BRDF.
        float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
        float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
        float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
        float3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
        float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
        directLighting += (((diffuseBRDF + specularBRDF) * Lradiance * cosLi)) * att * att * Lights[i].Intensity;
    }

	//// Ambient lighting (IBL).
 //   float3 ambientLighting;
	//{
	//	// Sample diffuse irradiance at normal direction.
 //       float3 irradiance = irradianceTexture.Sample(gsamLinear, N).rgb;

	//	// Calculate Fresnel term for ambient lighting.
	//	// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
	//	// use cosLo instead of angle with light's half-vector (cosLh above).
	//	// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
 //       float3 F = fresnelSchlick(F0, cosLo);

	//	// Get diffuse contribution factor (as with direct lighting).
 //       float3 kd = lerp(1.0 - F, 0.0, metalness);

	//	// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
 //       float3 diffuseIBL = kd * albedo * irradiance;

	//	// Sample pre-filtered specular reflection environment at correct mipmap level.
 //       uint specularTextureLevels = querySpecularTextureLevels();
 //       float3 specularIrradiance = specularTexture.SampleLevel(gsamLinear, Lr, roughness * specularTextureLevels).rgb;

	//	// Split-sum approximation factors for Cook-Torrance specular BRDF.
 //       float2 specularBRDF = specularBRDF_LUT.Sample(gsamLinear, float2(cosLo, roughness)).rg;

	//	// Total specular IBL contribution.
 //       float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

	//	// Total ambient lighting contribution.
 //       ambientLighting = diffuseIBL + specularIBL;
 //   }

    directLighting += pow(gEmissionMap.Sample(gsamLinear, pin.texcoord).rgb, 2.2f);
    directLighting += pow(gMaterial.Emission.rgb, 2.2f);
   
    //directLighting += gEmissionMap.Sample(gsamLinear, pin.texcoord).rgb;
    //directLighting += gMaterial.Emission.rgb;
    
    directLighting = pow(directLighting, 1 / 2.2f);
    float luma = (0.2126 * directLighting.r + 0.7152 * directLighting.g + 0.0722 * directLighting.b);
    
	// Final fragment color.
    return float4(directLighting, luma);
}