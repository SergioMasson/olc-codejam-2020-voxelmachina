#pragma once

#include "texture2D.h"

namespace graphics
{
	class RenderPipeline;

	class Material
	{
	public:
		Material()
		{
			ZeroMemory(this, sizeof(this));
			Emission = { 0.0f, 0.0f, 0.0f, 0.0f };
		}

		DirectX::XMFLOAT4 Color;
		DirectX::XMFLOAT4 Emission;

		float Metalness;
		float Roughness;
		DirectX::XMFLOAT2 Pad;

		friend RenderPipeline;
	};
}