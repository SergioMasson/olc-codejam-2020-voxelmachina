#include "pch.h"
#include "FXAA.h"
#include "../coreGraphics.h"
#include "../../colors.h"

#include "FXAA_PS.h"
#include "FXAA_VS.h"

struct CB_FXAA
{
	DirectX::XMFLOAT4 m_fxaa;
};

Microsoft::WRL::ComPtr<ID3D11Buffer> g_pcbFXAA = NULL;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShaderFXAA = NULL;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderFXAA = NULL;

//Texture where the scene should be rendered in order to perform FXAA
Microsoft::WRL::ComPtr<ID3D11Texture2D> g_pProxyTexture = NULL;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> graphics::FXAA::g_pProxyTextureRTV = NULL;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_pProxyTextureSRV = NULL;

Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamPointMirror = NULL;
Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamLinearWrap = NULL;
Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamPointCmpClamp = NULL;
Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamAni = NULL;

Microsoft::WRL::ComPtr <ID3D11BlendState> g_pColorWritesOn = NULL;
Microsoft::WRL::ComPtr <ID3D11BlendState> g_pColorWritesOff = NULL;

Microsoft::WRL::ComPtr <ID3D11RasterizerState> g_pCullBack = NULL;
Microsoft::WRL::ComPtr <ID3D11RasterizerState > g_pCullFront = NULL;

bool graphics::FXAA::FXAAEnable = true;

void CreateRasterazerStates()
{
	// Create blend state objects
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateBlendState(&blendDesc, g_pColorWritesOn.GetAddressOf()));

	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateBlendState(&blendDesc, g_pColorWritesOff.GetAddressOf()));

	// Create rasterizer state objects
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.AntialiasedLineEnable = FALSE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.MultisampleEnable = TRUE;
	rastDesc.ScissorEnable = FALSE;
	rastDesc.SlopeScaledDepthBias = 0;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateRasterizerState(&rastDesc, g_pCullBack.GetAddressOf()));

	rastDesc.CullMode = D3D11_CULL_FRONT;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateRasterizerState(&rastDesc, g_pCullFront.GetAddressOf()));
}

void CreateSampleStates()
{
	// Create sampler objects
	D3D11_SAMPLER_DESC samDesc;

	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samDesc.MipLODBias = 0.0f;
	samDesc.MaxAnisotropy = 1;
	samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samDesc.BorderColor[0] = samDesc.BorderColor[1] = samDesc.BorderColor[2] = samDesc.BorderColor[3] = 0;
	samDesc.MinLOD = 0;
	samDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateSamplerState(&samDesc, g_pSamPointMirror.GetAddressOf()));

	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateSamplerState(&samDesc, g_pSamLinearWrap.GetAddressOf()));

	samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateSamplerState(&samDesc, g_pSamPointCmpClamp.GetAddressOf()));

	samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc.MaxAnisotropy = 4;
	samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samDesc.MaxLOD = 0.0f;
	samDesc.MinLOD = 0.0f;
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateSamplerState(&samDesc, g_pSamAni.GetAddressOf()));
}

void CreateFXAAConstBuffer()
{
	// Create constant buffer
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	cbDesc.ByteWidth = sizeof(CB_FXAA);
	ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateBuffer(&cbDesc, NULL, g_pcbFXAA.GetAddressOf()), "fail to create FXAA const buffer.");
}

void CreateFXAAShaders()
{
	auto hr = graphics::g_d3dDevice->CreateVertexShader(g_pFXAA_VS, sizeof(g_pFXAA_VS), 0, g_pVertexShaderFXAA.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create vertex shader.");

	hr = graphics::g_d3dDevice->CreatePixelShader(g_pFXAA_PS, sizeof(g_pFXAA_PS), 0, g_pPixelShaderFXAA.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");
}

void FxaaIntegrateResource(UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.Height = height;
	desc.Width = width;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	auto hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_pProxyTexture.GetAddressOf());

	ASSERT_SUCCEEDED(hr);

	hr = graphics::g_d3dDevice->CreateRenderTargetView(g_pProxyTexture.Get(), 0, graphics::FXAA::g_pProxyTextureRTV.GetAddressOf());

	ASSERT_SUCCEEDED(hr);

	hr = graphics::g_d3dDevice->CreateShaderResourceView(g_pProxyTexture.Get(), 0, g_pProxyTextureSRV.GetAddressOf());

	ASSERT_SUCCEEDED(hr);
}

void graphics::FXAA::Initialize()
{
	CreateFXAAConstBuffer();
	CreateFXAAShaders();
	CreateSampleStates();
	CreateRasterazerStates();
}

void graphics::FXAA::Resize(UINT width, UINT height)
{
	if (g_pProxyTexture != nullptr)
		g_pProxyTexture.Reset();

	if (g_pProxyTextureRTV != nullptr)
		g_pProxyTextureRTV.Reset();

	if (g_pProxyTextureSRV != nullptr)
		g_pProxyTextureSRV.Reset();

	FxaaIntegrateResource(width, height);

	D3D11_MAPPED_SUBRESOURCE MappedResource;

	auto hr = (graphics::g_d3dImmediateContext->Map(g_pcbFXAA.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	ASSERT_SUCCEEDED(hr, "Fail to map FXAA resource");

	CB_FXAA* pFXAA = (CB_FXAA*)MappedResource.pData;
	float frameWidth = (float)graphics::g_windowWidth;
	float frameHeight = (float)graphics::g_windowHeight;
	pFXAA->m_fxaa = DirectX::XMFLOAT4(1.0f / frameWidth, 1.0f / frameHeight, 0.0f, 0.0f);
	graphics::g_d3dImmediateContext->Unmap(g_pcbFXAA.Get(), 0);
}

void graphics::FXAA::Shutdown()
{
	g_pcbFXAA.Reset();
	g_pProxyTexture.Reset();
	g_pProxyTextureRTV.Reset();
	g_pProxyTextureSRV.Reset();
	g_pVertexShaderFXAA.Reset();
	g_pPixelShaderFXAA.Reset();
}

void graphics::FXAA::Render(ID3D11RenderTargetView* const* target)
{
	ID3D11SamplerState* ppSamplerStates[4] = { g_pSamPointMirror.Get(), g_pSamLinearWrap.Get(), g_pSamPointCmpClamp.Get(), g_pSamAni.Get() };
	graphics::g_d3dImmediateContext->PSSetSamplers(0, 4, ppSamplerStates);

	graphics::g_d3dImmediateContext->RSSetState(g_pCullBack.Get());
	graphics::g_d3dImmediateContext->PSSetConstantBuffers(1, 1, g_pcbFXAA.GetAddressOf());

	graphics::g_d3dImmediateContext->OMSetRenderTargets(1, target, nullptr);
	graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderFXAA.Get(), NULL, 0);
	graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderFXAA.Get(), NULL, 0);
	graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_pProxyTextureSRV.GetAddressOf());
	graphics::g_d3dImmediateContext->Draw(4, 0);

	graphics::g_d3dImmediateContext->ClearRenderTargetView(g_pProxyTextureRTV.Get(), reinterpret_cast<const float*>(&Color::Black));
}