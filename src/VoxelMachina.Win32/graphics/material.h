#pragma once

#include "texture2D.h"

namespace graphics
{
	class RenderPipeline;

	class Material
	{
	public:
		Material() { ZeroMemory(this, sizeof(this)); }

		DirectX::XMFLOAT4 Color;
		DirectX::XMFLOAT4 Emission;

		float Diffuse;
		float Specular; // w = SpecPower
		DirectX::XMFLOAT2 Pad;

		friend RenderPipeline;
	};
}