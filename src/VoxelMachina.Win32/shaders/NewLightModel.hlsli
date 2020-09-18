#define MAX_LIGHTS 10
#define DIR_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2

struct Light
{
    float4 Color;
    
    float3 Position;
    float Ambient;
    
    float3 Direction;
    float Intensity;
    
    float Range;
    float Spot;
    int LightType;
    float Pad;
};

struct Material
{
    float4 Color;
    
    float4 Emission;
    
    float Diffuse;
    float Specular; // w = SpecPower
    float2 Pad;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    float atenuation = saturate((falloffEnd - d) / (falloffEnd - falloffStart));
    
    // Linear falloff.
    return atenuation;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
float4 ComputeDirectionalLight(Material mat, Light L, float3 normal, float3 toEye)
{
    float ambient = 0.0f;
    float diffuse = 0.0f;
    float spec = 0.0f;

	// The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

	// Add ambient term.
    ambient = L.Ambient;

    normal = normalize(normal);
    
	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
    float diffuseFactor = max(dot(normal, lightVec), 0.0f);

    float3 v = reflect(-lightVec, normal);
    spec = pow(max(dot(v, toEye), 0.0f), mat.Specular);
    diffuse = diffuseFactor * mat.Diffuse;
    
    return max((ambient + diffuse + spec) * L.Intensity * L.Color, 0.0f);
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
float4 ComputePointLight(Material mat, Light L, float3 pos, float3 normal, float3 toEye)
{
	// Initialize outputs.
    float ambient = 0.0f;
    float diffuse = 0.0f;
    float spec = 0.0f;

	// The vector from the surface to the light.
    float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
    float d = length(lightVec);
	
	// Range test.
    if (d > L.Range)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
		
	// Normalize the light vector.
    lightVec /= d;
	
	// Ambient term.
    ambient = L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

    float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.

    float3 v = reflect(-lightVec, normal);
    spec = pow(max(dot(v, toEye), 0.0f), mat.Specular);
    diffuse = diffuseFactor * mat.Diffuse;

	// Attenuate
    float att = CalcAttenuation(d, 0, L.Range);

    return max((diffuse + spec + ambient) * L.Intensity * att * L.Color, 0.0f);
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
float4 ComputeSpotLight(Material mat, Light L, float3 pos, float3 normal, float3 toEye)
{
	// Initialize outputs.
    float ambient = 0.0f;
    float diffuse = 0.0f;
    float spec = 0.0f;

	// The vector from the surface to the light.
    float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
    float d = length(lightVec);
	
	// Range test.
    if (d > L.Range)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
		
	// Normalize the light vector.
    lightVec /= d;
	
	// Ambient term.
    ambient =  L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

    float diffuseFactor = dot(lightVec, normal);

    float3 v = reflect(-lightVec, normal);
    spec = pow(max(dot(v, toEye), 0.0f), mat.Specular);
    diffuse =  diffuseFactor * mat.Diffuse;
	
	// Scale by spotlight factor and attenuate.
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    float att = CalcAttenuation(d, 0, L.Range);
    
    return max(((ambient + diffuse + spec) * L.Intensity * att) * L.Color, 0.0f);
}