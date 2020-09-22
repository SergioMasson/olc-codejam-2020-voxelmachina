#include "pch.h"
#include "postProcessingEffects.h"
#include "../coreGraphics.h"
#include "FXAA.h"
#include "Bloom.h"

void graphics::PostEffects::Initialize()
{
	FXAA::Initialize();
	bloom::Initialize();
}

void graphics::PostEffects::Render()
{
	if (graphics::FXAA::FXAAEnable)
	{
		if (graphics::bloom::Enable)
			graphics::FXAA::Render(graphics::bloom::g_inputRenderTarget.GetAddressOf());
		else
			graphics::FXAA::Render(graphics::g_ScreenRenderTargetView.GetAddressOf());
	}

	if (graphics::bloom::Enable)
	{
		graphics::bloom::Render(graphics::g_ScreenRenderTargetView.GetAddressOf());
	}
}

void graphics::PostEffects::Resize(UINT width, UINT height)
{
	graphics::FXAA::Resize(width, height);
	graphics::bloom::Resize(width, height);
}

void graphics::PostEffects::Shutdown()
{
	FXAA::Shutdown();
	bloom::Shutdown();
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> graphics::PostEffects::GetInitialRenderTarget()
{
	if (FXAA::FXAAEnable)
		return FXAA::g_pProxyTextureRTV;

	if (bloom::Enable)
		return bloom::g_inputRenderTarget;

	return graphics::g_ScreenRenderTargetView;
}