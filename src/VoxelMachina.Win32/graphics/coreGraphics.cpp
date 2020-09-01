#include "pch.h"
#include "coreGraphics.h"
#include "../application.h"
#include "../dxassert.h"
#include "../colors.h"

using namespace Microsoft::WRL;
using namespace graphics;

UINT							graphics::g_4xMsaaQuality;
LONG							graphics::g_windowWidth;
LONG							graphics::g_windowHeight;

D3D11_VIEWPORT					graphics::g_ScreenViewport;
D3D_DRIVER_TYPE					graphics::g_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
bool							graphics::g_Enable4xMsaa;

ComPtr<ID3D11RenderTargetView>	graphics::g_RenderTargetView = nullptr;
ComPtr<ID3D11DepthStencilView>	graphics::g_DepthStencilView = nullptr;
ComPtr<ID3D11Device>			graphics::g_d3dDevice = nullptr;
ComPtr<ID3D11DeviceContext>		graphics::g_d3dImmediateContext = nullptr;
ComPtr<IDXGISwapChain>			graphics::g_SwapChain = nullptr;
ComPtr<ID3D11Texture2D>			graphics::g_DepthStencilBuffer = nullptr;

void InitializeGraphicsInfra(uint32_t width, uint32_t heigth)
{
	// Create the device and device context.
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;

	//Create the D3D Device (GPU)
	HRESULT hr = D3D11CreateDevice(0, g_d3dDriverType, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &g_d3dDevice, &featureLevel, &g_d3dImmediateContext);

	//Check if creation succeed.
	ASSERT_SUCCEEDED(hr, "Fail to create D3DDevice");

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		ASSERT(false, "Direct3D Feature Level 11 unsupported.");
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render
	// target formats, so we only need to check quality support.
	ASSERT_SUCCEEDED(g_d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &g_4xMsaaQuality));

	ASSERT(g_4xMsaaQuality > 0, "4xMSAA not suported.");

	g_windowWidth = width;
	g_windowHeight = heigth;

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = g_windowWidth;
	sd.BufferDesc.Height = g_windowHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA?
	if (g_Enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = g_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = g_coreWindow;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain:
	// This function is being called with a device from a different IDXGIFactory."

	ComPtr<IDXGIDevice> dxgiDevice;
	ASSERT_SUCCEEDED(g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf()));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ASSERT_SUCCEEDED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory = 0;
	ASSERT_SUCCEEDED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.GetAddressOf()));

	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChain(g_d3dDevice.Get(), &sd, &g_SwapChain));
}

void graphics::Initialize(uint32_t width, uint32_t heigth)
{
	InitializeGraphicsInfra(width, heigth);
	graphics::Resize(g_windowWidth, g_windowHeight);
}

void graphics::Resize(uint32_t width, uint32_t heigth)
{
	if (width == 0 || heigth == 0)
		return;

	g_windowWidth = width;
	g_windowHeight = heigth;

	ASSERT(g_d3dImmediateContext);
	ASSERT(g_d3dDevice);
	ASSERT(g_SwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	g_RenderTargetView.Reset();
	g_DepthStencilView.Reset();
	g_DepthStencilBuffer.Reset();

	// Resize the swap chain and recreate the render target view.

	ASSERT_SUCCEEDED(g_SwapChain->ResizeBuffers(1, g_windowWidth, g_windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	ID3D11Texture2D* backBuffer;

	ASSERT_SUCCEEDED(g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	ASSERT_SUCCEEDED(g_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, g_RenderTargetView.GetAddressOf()));

	backBuffer->Release();

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = g_windowWidth;
	depthStencilDesc.Height = g_windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (g_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = g_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ASSERT_SUCCEEDED(g_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, g_DepthStencilBuffer.GetAddressOf()));
	ASSERT_SUCCEEDED(g_d3dDevice->CreateDepthStencilView(g_DepthStencilBuffer.Get(), nullptr, g_DepthStencilView.GetAddressOf()));

	// Bind the render target view and depth/stencil view to the pipeline.
	g_d3dImmediateContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), g_DepthStencilView.Get());

	// Set the viewport transform.
	g_ScreenViewport.TopLeftX = 0;
	g_ScreenViewport.TopLeftY = 0;
	g_ScreenViewport.Width = static_cast<float>(g_windowWidth);
	g_ScreenViewport.Height = static_cast<float>(g_windowHeight);
	g_ScreenViewport.MinDepth = 0.0f;
	g_ScreenViewport.MaxDepth = 1.0f;

	g_d3dImmediateContext->RSSetViewports(1, &g_ScreenViewport);
}

void graphics::BeginDraw()
{
	g_d3dImmediateContext->ClearRenderTargetView(g_RenderTargetView.Get(), reinterpret_cast<const float*>(&colors::black));
	g_d3dImmediateContext->ClearDepthStencilView(g_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void graphics::Present()
{
	ASSERT_SUCCEEDED(g_SwapChain->Present(0, 0));
}

void graphics::ShutDown()
{
}