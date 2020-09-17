#pragma once

namespace graphics
{
	namespace FXAA
	{
		void Initialize();
		void Resize(UINT width, UINT height);
		void Shutdown();
		void Render();

		extern bool FXAAEnable;
		extern Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_pProxyTextureRTV;
	}
}