#pragma once

struct Light
{
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Position;
	float Ambient;
	DirectX::XMFLOAT3 Direction;
	float Intensity;
	float Range;
	float Spot;
	DirectX::XMFLOAT2 Pad;
};
