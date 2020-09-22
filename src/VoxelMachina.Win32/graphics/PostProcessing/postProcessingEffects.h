#pragma once

namespace graphics
{
	namespace PostEffects
	{
		void Initialize();
		void Render();
		void Resize(UINT width, UINT height);
		void Shutdown();

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetInitialRenderTarget();
	}
}