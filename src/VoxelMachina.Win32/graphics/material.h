#pragma once

namespace graphics
{
	class RenderPipeline;

	class Material
	{
	public:
		Material() { ZeroMemory(this, sizeof(this)); }

		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular; // w = SpecPower
		DirectX::XMFLOAT4 Reflect;

		friend RenderPipeline;
	};
}