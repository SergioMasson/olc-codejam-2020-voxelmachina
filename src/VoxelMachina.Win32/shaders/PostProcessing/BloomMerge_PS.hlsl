struct BloomVS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


cbuffer cbFlags : register(b0)
{
    uint horizontal;
    float exposure;
    float2 pad;
}

SamplerState InputSampler : register(s3);
Texture2D SceneTexture : register(t0);
Texture2D BlurTexture : register(t1);

static const float gamma = 2.2;

float4 main(BloomVS_Output Input) : SV_TARGET
{
    float3 hdrColor = SceneTexture.Sample(InputSampler, Input.Tex).rgb;
    float3 bloomColor = BlurTexture.Sample(InputSampler, Input.Tex).rgb;
    
    hdrColor += bloomColor; // additive blending

    // tone mapping
    float3 result = float3(1.0, 1.0, 1.0) - exp(-hdrColor * exposure);
    //float3 oneOverGama = float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma);
    
    // also gamma correct while we're at it       
    //result = pow(result, oneOverGama);
    return float4(result, 1.0);
}