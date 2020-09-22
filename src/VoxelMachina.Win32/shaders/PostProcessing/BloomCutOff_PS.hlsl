struct BloomVS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

cbuffer cbFxaa : register(b1)
{
    float4 RCPFrame : packoffset(c0);
};

SamplerState InputSampler : register(s3);
Texture2D InputTexture : register(t0);

float4 main(BloomVS_Output Input) : SV_TARGET
{
    float3 luma = float3(0.2126, 0.7152, 0.0722);
    float3 albedo = InputTexture.Sample(InputSampler, Input.Tex).xyz;
    
    float gray = dot(albedo, luma);
    
    if (gray > 1.0f)
        return float4(albedo, 1.0);
    
    return float4(0.0F, 0.0F, 0.0F, 1.0f);
}