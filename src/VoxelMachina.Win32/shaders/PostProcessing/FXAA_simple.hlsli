cbuffer cbFxaa : register(b1)
{
    float4 RCPFrame : packoffset(c0);
};

struct FxaaVS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

SamplerState InputSampler : register(s3);
Texture2D InputTexture : register(t0);

static const float R_fxaaSpanMax = 8.0f;
static const float R_fxaaReduceMin = 1.0f / 128.0f;
static const float R_fxaaReduceMul = 1.0f / 8.0f;

float4 main(FxaaVS_Output Input) : SV_TARGET
{
    float2 texCoordOffset = RCPFrame.xy;
	
    float3 luma = float3(0.299, 0.587, 0.114);
    
    InputTexture.Sample(InputSampler, (float2(-1.0, -1.0) * texCoordOffset)).xyz;
    
    float lumaTL = dot(luma, InputTexture.Sample(InputSampler, Input.Tex + (float2(-1.0, -1.0) * texCoordOffset)).xyz);
    float lumaTR = dot(luma, InputTexture.Sample(InputSampler, Input.Tex + (float2(1.0, -1.0) * texCoordOffset)).xyz);
    float lumaBL = dot(luma, InputTexture.Sample(InputSampler, Input.Tex + (float2(-1.0, 1.0) * texCoordOffset)).xyz);
    float lumaBR = dot(luma, InputTexture.Sample(InputSampler, Input.Tex + (float2(1.0, 1.0) * texCoordOffset)).xyz);
    float lumaM = dot(luma, InputTexture.Sample(InputSampler,  Input.Tex).xyz);
    
    float2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));
	
    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (R_fxaaReduceMul * 0.25), R_fxaaReduceMin);
    float inverseDirAdjustment = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	
    dir = min(float2(R_fxaaSpanMax, R_fxaaSpanMax),
		max(float2(-R_fxaaSpanMax, -R_fxaaSpanMax), dir * inverseDirAdjustment)) * texCoordOffset;

    float3 result1 = (1.0 / 2.0) * (
    InputTexture.Sample(InputSampler, Input.Tex.xy + (dir * float2(1.0 / 3.0 - 0.5, 1.0 / 3.0 - 0.5))).xyz +
    InputTexture.Sample(InputSampler, Input.Tex.xy + (dir * float2(2.0 / 3.0 - 0.5, 2.0 / 3.0 - 0.5))).xyz);

    float3 result2 = result1 * (1.0 / 2.0) + (1.0 / 4.0) * (
    InputTexture.Sample(InputSampler, Input.Tex.xy + (dir * float2(0.0 / 3.0 - 0.5, 0.0 / 3.0 - 0.5))).xyz +
    InputTexture.Sample(InputSampler, Input.Tex.xy + (dir * float2(3.0 / 3.0 - 0.5, 3.0 / 3.0 - 0.5))).xyz);

    float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
    float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
    float lumaResult2 = dot(luma, result2);
	
    if (lumaResult2 < lumaMin || lumaResult2 > lumaMax)
        return float4(result1, 1.0);
    else
        return float4(result2, 1.0);
}