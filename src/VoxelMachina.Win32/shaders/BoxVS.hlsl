cbuffer cbPerObject : register(b0)
{
	float4x4 modelMatrix;
};

cbuffer cbScene : register(b1)
{
	float4x4 viewProjectMatrix;
};

struct Input
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Output main(Input input)
{
	Output output;

	output.position = float4(input.position.x, input.position.y, input.position.z, 1);

	float4x4 mvpMatrix = mul(viewProjectMatrix, modelMatrix);

	output.position = mul(output.position, transpose(mvpMatrix));
	output.color = input.color;

	return output;
}