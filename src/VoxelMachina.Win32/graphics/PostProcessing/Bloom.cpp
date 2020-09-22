#include "pch.h"
#include "Bloom.h"
#include "../coreGraphics.h"

#include "Bloom_VS.h"
#include "Bloom_PS.h"

bool graphics::bloom::Enable = true;

//Cutoff shaders
static Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShaderBloomCutoff = NULL;
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloomCutoff = NULL;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_cutoffTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_cutoffTextureSRV = NULL;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_cutoffTextureRTV = NULL;

//Blur shaders
static Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShaderBloomBlur = NULL;
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloomBlur = NULL;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_blurTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_blurTextureSRV = NULL;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_blurTextureRTV = NULL;

//Merge shaders
static Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShaderBloom = NULL;
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloom = NULL;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_pProxyTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamAni = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_pProxyTextureSRV = NULL;

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> graphics::bloom::g_inputRenderTarget = NULL;

namespace graphics
{
	void CreateMergeTexture(UINT width, UINT height)
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
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_pProxyTexture.Get(), 0, bloom::g_inputRenderTarget.GetAddressOf());

		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_pProxyTexture.Get(), 0, g_pProxyTextureSRV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
	}

	void CreateCutoffTexture(UINT width, UINT height)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = height;
		desc.Width = width;
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		auto hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_blurTexture.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_blurTexture.Get(), 0, g_cutoffTextureRTV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_blurTexture.Get(), 0, g_cutoffTextureSRV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
	}

	void CreateBlurTexture(UINT width, UINT height)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = height;
		desc.Width = width;
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		auto hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_cutoffTexture.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_cutoffTexture.Get(), 0, g_blurTextureRTV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_cutoffTexture.Get(), 0, g_blurTextureSRV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
	}

	void bloom::Initialize()
	{
		// Create sampler objects
		D3D11_SAMPLER_DESC samDesc;

		ZeroMemory(&samDesc, sizeof(samDesc));
		samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samDesc.MipLODBias = 0.0f;
		samDesc.MaxAnisotropy = 4;
		samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samDesc.BorderColor[0] = samDesc.BorderColor[1] = samDesc.BorderColor[2] = samDesc.BorderColor[3] = 0;
		samDesc.MaxLOD = 0.0f;
		samDesc.MinLOD = 0.0f;

		ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateSamplerState(&samDesc, g_pSamAni.GetAddressOf()));

		auto hr = graphics::g_d3dDevice->CreateVertexShader(g_pBloom_VS, sizeof(g_pBloom_VS), 0, g_pVertexShaderBloom.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create vertex shader.");

		hr = graphics::g_d3dDevice->CreatePixelShader(g_pBloom_PS, sizeof(g_pBloom_PS), 0, g_pPixelShaderBloom.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");
	}

	void bloom::Resize(UINT width, UINT height)
	{
		CreateMergeTexture(width, height);
		CreateCutoffTexture(width, height);
		CreateBlurTexture(width, height);
	}

	void bloom::Shutdown()
	{
		g_pVertexShaderBloom.Reset();
		g_pPixelShaderBloom.Reset();
		g_pProxyTexture.Reset();
		g_inputRenderTarget.Reset();
		g_pProxyTextureSRV.Reset();

		g_pVertexShaderBloomCutoff.Reset();
		g_pPixelShaderBloomCutoff.Reset();
		g_cutoffTexture.Reset();
		g_cutoffTextureSRV.Reset();
		g_cutoffTextureRTV.Reset();

		g_pVertexShaderBloomBlur.Reset();
		g_pPixelShaderBloomBlur.Reset();
		g_blurTexture.Reset();
		g_blurTextureSRV.Reset();
		g_blurTextureRTV.Reset();
	}

	void bloom::Render(ID3D11RenderTargetView* const* target)
	{
		graphics::g_d3dImmediateContext->OMSetRenderTargets(1, target, nullptr);
		graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_pProxyTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->Draw(4, 0);
	}
}