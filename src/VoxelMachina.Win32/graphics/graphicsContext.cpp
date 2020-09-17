#include "pch.h"
#include "graphicsContext.h"
#include "coreGraphics.h"

void graphics::GraphicsContext::SetRenderTargets(UINT NumRTVs, ID3D11RenderTargetView* const* renderTargets)
{
	g_d3dImmediateContext->OMSetRenderTargets(NumRTVs, renderTargets, nullptr);
}

void graphics::GraphicsContext::SetRenderTargets(UINT NumRTVs, ID3D11RenderTargetView* const* renderTargets, ID3D11DepthStencilView* DSV)
{
	g_d3dImmediateContext->OMSetRenderTargets(NumRTVs, renderTargets, DSV);
}

void graphics::GraphicsContext::ClearColor(ID3D11RenderTargetView* Target, Color& color)
{
	g_d3dImmediateContext->ClearRenderTargetView(Target, reinterpret_cast<const float*>(&color));
}

void graphics::GraphicsContext::ClearDepth(ID3D11DepthStencilView* Target)
{
	g_d3dImmediateContext->ClearDepthStencilView(Target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void graphics::GraphicsContext::ClearStencil(ID3D11DepthStencilView* Target)
{
	g_d3dImmediateContext->ClearDepthStencilView(Target, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void graphics::GraphicsContext::ClearDepthAndStencil(ID3D11DepthStencilView* Target)
{
	g_d3dImmediateContext->ClearDepthStencilView(Target, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}