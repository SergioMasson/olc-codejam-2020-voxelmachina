#include "pch.h"
#include "coreGraphics.h"
#include "../application.h"
#include "../dxassert.h"
#include "../colors.h"

#include "PostProcessing/PostProcessingEffects.h"

using namespace Microsoft::WRL;
using namespace graphics;

UINT							graphics::g_4xMsaaQuality;
LONG							graphics::g_windowWidth;
LONG							graphics::g_windowHeight;

D3D11_VIEWPORT					graphics::g_ScreenViewport;
D3D_DRIVER_TYPE					graphics::g_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
bool							graphics::g_Enable4xMsaa = false;

ComPtr<ID3D11RenderTargetView>  graphics::g_ScreenRenderTargetView;
ComPtr<ID3D11DepthStencilView>	graphics::g_ScreenDepthStencilView = nullptr;

ComPtr<ID3D11Device>			graphics::g_d3dDevice = nullptr;
ComPtr<ID3D11DeviceContext>		graphics::g_d3dImmediateContext = nullptr;
ComPtr<IDXGISwapChain1>			graphics::g_SwapChain = nullptr;
ComPtr<ID3D11Texture2D>			graphics::g_DepthStencilBuffer = nullptr;

ComPtr<ID2D1Device>				graphics::g_d2dDevice = nullptr;
ComPtr<ID2D1DeviceContext>		graphics::g_d2dDeviceContext = nullptr;
ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;

Color m_cleanColor{ Color::Black };

void createD2DRenderTarget(IDXGISurface1* ptr)
{
	// specify the desired bitmap properties
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bp.dpiX = 96.0f;
	bp.dpiY = 96.0f;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = nullptr;

	//ERROR(Sergio): This method breaks when called on resize.
	ASSERT_SUCCEEDED(g_d2dDeviceContext->CreateBitmapFromDxgiSurface(ptr, &bp, d2dRenderTarget.GetAddressOf()));

	// set the newly created bitmap as render target
	g_d2dDeviceContext->SetTarget(d2dRenderTarget.Get());
}

void createD3DRenderTarget(ComPtr<IDXGISurface1> ptr)
{
	ComPtr<ID3D11Resource> d3dbackBuffer;

	ptr.As(&d3dbackBuffer);

	ASSERT_SUCCEEDED(g_d3dDevice->CreateRenderTargetView(d3dbackBuffer.Get(), nullptr, g_ScreenRenderTargetView.GetAddressOf()));

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = g_windowWidth;
	depthStencilDesc.Height = g_windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create a texture for depth and stencil
	ASSERT_SUCCEEDED(g_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, g_DepthStencilBuffer.GetAddressOf()));

	//Create a view to the depth and stencil buffer.
	ASSERT_SUCCEEDED(g_d3dDevice->CreateDepthStencilView(g_DepthStencilBuffer.Get(), nullptr, g_ScreenDepthStencilView.GetAddressOf()));

	// Set the viewport transform.
	g_ScreenViewport.TopLeftX = 0;
	g_ScreenViewport.TopLeftY = 0;
	g_ScreenViewport.Width = static_cast<float>(g_windowWidth);
	g_ScreenViewport.Height = static_cast<float>(g_windowHeight);
	g_ScreenViewport.MinDepth = 0.0f;
	g_ScreenViewport.MaxDepth = 1.0f;

	//Set the view port to the entire buffer.
	g_d3dImmediateContext->RSSetViewports(1, &g_ScreenViewport);
}

void InitializeGraphicsInfra(uint32_t width, uint32_t heigth)
{
	// Create the device and device context.
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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

	ComPtr<IDXGIDevice1> dxgiDevice;
	// Obtain the underlying DXGI device of the Direct3D11 device.
	ASSERT_SUCCEEDED(g_d3dDevice.As(&dxgiDevice));

	ComPtr<ID2D1Factory1> m_d2dFactory;

#if defined(DEBUG) || defined(_DEBUG)
	D2D1_FACTORY_OPTIONS options;
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, m_d2dFactory.GetAddressOf());

#else
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.GetAddressOf());
#endif

	// Create a Direct2D factory.

	// Obtain the Direct2D device for 2-D rendering.
	ASSERT_SUCCEEDED(m_d2dFactory->CreateDevice(dxgiDevice.Get(), g_d2dDevice.GetAddressOf()));

	// Get Direct2D device's corresponding device context object.
	ASSERT_SUCCEEDED(g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, g_d2dDeviceContext.GetAddressOf()));

	g_windowWidth = width;
	g_windowHeight = heigth;

	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = width;                           // use automatic sizing
	swapChainDesc.Height = heigth;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.Stereo = false;

	// Use 4X MSAA? --must match swap chain MSAA values.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SwapChainBufferCount; // use double buffering to enable flip
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
	swapChainDesc.Flags = 0;

	// Identify the physical adapter (GPU or card) this device is runs on.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ASSERT_SUCCEEDED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

	// Get the factory object that created the DXGI device.
	ComPtr<IDXGIFactory2> dxgiFactory;
	ASSERT_SUCCEEDED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

	// Get the final swap chain for this window from the DXGI factory.
	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForHwnd(g_d3dDevice.Get(), g_coreWindow, &swapChainDesc, nullptr, nullptr, g_SwapChain.GetAddressOf()));

	// Ensure that DXGI doesn't queue more than one frame at a time.
	ASSERT_SUCCEEDED(dxgiDevice->SetMaximumFrameLatency(1));

	Microsoft::WRL::ComPtr<IDXGISurface1> dxgiBuffer;
	ASSERT_SUCCEEDED(g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), &dxgiBuffer));

	createD3DRenderTarget(dxgiBuffer);
	createD2DRenderTarget(dxgiBuffer.Get());
}

void graphics::Initialize(uint32_t width, uint32_t heigth)
{
	InitializeGraphicsInfra(width, heigth);

	graphics::PostEffects::Initialize();
	graphics::PostEffects::Resize(g_windowWidth, g_windowHeight);
}

void graphics::Resize(uint32_t width, uint32_t heigth)
{
	if (width == 0 || heigth == 0)
		return;

	if (g_windowWidth == width && g_windowHeight == heigth)
		return;

	g_windowWidth = width;
	g_windowHeight = heigth;

	ASSERT(g_d3dImmediateContext);
	ASSERT(g_d3dDevice);
	ASSERT(g_SwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	g_ScreenRenderTargetView.Reset();
	g_ScreenDepthStencilView.Reset();
	g_DepthStencilBuffer.Reset();
	d2dRenderTarget.Reset();
	g_d2dDeviceContext->SetTarget(nullptr);

	// Resize the swap chain and recreate the render target view.

	ASSERT_SUCCEEDED(g_SwapChain->ResizeBuffers(SwapChainBufferCount, g_windowWidth, g_windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	ComPtr<IDXGISurface1> backBuffer;
	ASSERT_SUCCEEDED(g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), &backBuffer));

	ComPtr<IDXGIDevice1> dxgiDevice;
	// Obtain the underlying DXGI device of the Direct3D11 device.
	ASSERT_SUCCEEDED(g_d3dDevice.As(&dxgiDevice));

	createD3DRenderTarget(backBuffer);
	createD2DRenderTarget(backBuffer.Get());

	graphics::PostEffects::Resize(width, heigth);
}

void graphics::BeginDraw()
{
	g_d3dImmediateContext->ClearRenderTargetView(g_ScreenRenderTargetView.Get(), reinterpret_cast<const float*>(&m_cleanColor));
	g_d3dImmediateContext->ClearDepthStencilView(g_ScreenDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	g_d3dImmediateContext->OMSetRenderTargets(1, graphics::PostEffects::GetInitialRenderTarget().GetAddressOf(), g_ScreenDepthStencilView.Get());
}

void graphics::Present()
{
	ASSERT_SUCCEEDED(g_SwapChain->Present(1, 0));
}

void graphics::ShutDown()
{
	graphics::PostEffects::Shutdown();
}