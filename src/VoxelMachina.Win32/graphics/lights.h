#pragma once

#include "../colors.h"
#include "../math/frustum.h"

#define DIR_LIGHT 0
#define SPOT_LIGHT 1
#define POINT_LIGHT 2

struct LightData
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

inline LightData CreateDirectionalLight(Color color, DirectX::XMFLOAT3 direction, float Intensity, float Ambient)
{
	return LightData
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

inline LightData CreatePointLight(Color color, DirectX::XMFLOAT3 position, float Intensity, float Ambient, float range)
{
	return LightData
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

inline LightData CreateSpotLight(Color color, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 position, float Intensity, float Ambient, float range, float spot)
{
	return LightData
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

inline bool IsLightInFrustrum(LightData light, math::Frustum frustrum)
{
	if (light.Range == 0.0f)
		return true;

	return frustrum.IntersectSphere(math::BoundingSphere(light.Position, light.Range));
}
