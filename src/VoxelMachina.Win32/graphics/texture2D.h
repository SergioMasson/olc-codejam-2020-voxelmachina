#pragma once

namespace graphics
{
	class RenderPipeline;
	class MeshRenderer;

	class Texture2D
	{
	public:
		Texture2D(const wchar_t* filePath);
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_resourceView;
		Microsoft::WRL::ComPtr<ID3D11Resource> m_resource;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
		friend RenderPipeline;
		friend MeshRenderer;
	};
}