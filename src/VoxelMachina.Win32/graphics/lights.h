#pragma once

#include "../colors.h"
#include "../math/frustum.h"

#define DIR_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2

struct Light
{
	Color Color;
	DirectX::XMFLOAT3 Position;
	float Ambient;
	DirectX::XMFLOAT3 Direction;
	float Intensity;
	float Range;
	float Spot;
	uint32_t LightType;
	float Pad;
};

inline Light CreateDirectionalLight(Color color, DirectX::XMFLOAT3 direction, float Intensity, float Ambient)
{
	return Light
	{
		color,
		DirectX::XMFLOAT3{},
		Ambient,
		direction,
		Intensity,
		0.0f,
		0.0f,
		DIR_LIGHT,
		0.0f
	};
}

inline Light CreatePointLight(Color color, DirectX::XMFLOAT3 position, float Intensity, float Ambient, float range)
{
	return Light
	{
		color,
		position,
		Ambient,
		DirectX::XMFLOAT3{},
		Intensity,
		range,
		0.0f,
		POINT_LIGHT,
		0.0f
	};
}

inline Light CreateSpotLight(Color color, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 position, float Intensity, float Ambient, float range, float spot)
{
	return Light
	{
		color,
		position,
		Ambient,
		direction,
		Intensity,
		range,
		spot,
		SPOT_LIGHT,
		0.0f
	};
}

inline bool IsLightInFrustrum(Light light, math::Frustum frustrum)
{
	if (light.Range == 0.0f)
		return true;

	return frustrum.IntersectSphere(math::BoundingSphere(light.Position, light.Range));
}