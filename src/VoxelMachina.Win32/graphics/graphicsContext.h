#pragma once

#include "../colors.h"

namespace graphics
{
	class GraphicsContext
	{
	public:
		void SetRenderTargets(UINT NumRTVs, ID3D11RenderTargetView* const*);
		void SetRenderTargets(UINT NumRTVs, ID3D11RenderTargetView* const*, ID3D11DepthStencilView* DSV);
		void SetRenderTarget(ID3D11RenderTargetView* RTV) { SetRenderTargets(1, &RTV); }
		void SetRenderTarget(ID3D11RenderTargetView* RTV, ID3D11DepthStencilView* DSV) { SetRenderTargets(1, &RTV, DSV); }
		void SetDepthStencilTarget(ID3D11DepthStencilView* DSV) { SetRenderTargets(0, nullptr, DSV); }

		void ClearColor(ID3D11RenderTargetView* Target, Color& color);
		void ClearDepth(ID3D11DepthStencilView* Target);
		void ClearStencil(ID3D11DepthStencilView* Target);
		void ClearDepthAndStencil(ID3D11DepthStencilView* Target);
	};
}
