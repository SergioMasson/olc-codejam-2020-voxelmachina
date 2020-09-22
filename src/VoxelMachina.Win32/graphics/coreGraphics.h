#pragma once

namespace graphics
{
	static const int SwapChainBufferCount = 3;

	void Initialize(uint32_t width, uint32_t heigth);
	void Resize(uint32_t width, uint32_t heigth);
	void BeginDraw();
	void Present();
	void ShutDown();

	extern UINT					g_4xMsaaQuality;
	extern LONG					g_windowWidth;
	extern LONG					g_windowHeight;
	extern D3D11_VIEWPORT g_ScreenViewport;
	extern D3D_DRIVER_TYPE g_d3dDriverType;
	extern bool g_Enable4xMsaa;

	extern Microsoft::WRL::ComPtr<ID3D11Device> g_d3dDevice;
	extern Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_d3dImmediateContext;
	extern Microsoft::WRL::ComPtr<IDXGISwapChain1> g_SwapChain;
	extern Microsoft::WRL::ComPtr<ID3D11Texture2D> g_DepthStencilBuffer;

	extern Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_ScreenRenderTargetView;
	extern Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_ScreenDepthStencilView;

	//D2DDevices
	extern Microsoft::WRL::ComPtr<ID2D1Device> g_d2dDevice;
	extern Microsoft::WRL::ComPtr<ID2D1DeviceContext> g_d2dDeviceContext;
}
