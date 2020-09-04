// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap : register(t0);
SamplerState gsamTriLinearSam : register(s0);

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};


float4 main(VertexOut pin) : SV_TARGET
{
    return gCubeMap.Sample(gsamTriLinearSam, pin.PosL);
}