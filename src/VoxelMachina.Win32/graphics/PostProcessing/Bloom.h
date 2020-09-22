#pragma once

namespace graphics
{
	namespace bloom
	{
		void Initialize();
		void Resize(UINT width, UINT height);
		void Shutdown();
		void Render(ID3D11RenderTargetView* const* target);

		extern bool Enable;
		extern float Exposure;
		extern Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_inputRenderTarget;
	}
}
