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

cbuffer cbFxaa : register(b1)
{
    float4 RCPFrame : packoffset(c0);
};

Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s3);

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 main(BloomVS_Output Input) : SV_TARGET
{
    float2 tex_offset = RCPFrame.xy; // gets size of single texel
    
    float3 result = InputTexture.Sample(InputSampler, Input.Tex).xyz * weight[0]; // current fragment's contribution
    
    if (horizontal == 1)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += InputTexture.Sample(InputSampler, Input.Tex + float2(tex_offset.x * i, 0.0)).xyz * weight[i];
            result += InputTexture.Sample(InputSampler, Input.Tex - float2(tex_offset.x * i, 0.0)).xyz * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += InputTexture.Sample(InputSampler, Input.Tex + float2(0.0, tex_offset.y * i)).xyz * weight[i];
            result += InputTexture.Sample(InputSampler, Input.Tex - float2(0.0, tex_offset.y * i)).xyz * weight[i];
        }
    }
    
    return float4(result, 1.0);
}