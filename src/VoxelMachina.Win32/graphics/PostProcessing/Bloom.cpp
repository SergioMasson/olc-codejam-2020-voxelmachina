#include "pch.h"
#include "Bloom.h"
#include "../coreGraphics.h"

#include "Bloom_VS.h"
#include "BloomBlur_PS.h"
#include "BloomCutoff_PS.h"
#include "BloomMerge_PS.h"

bool graphics::bloom::Enable = true;
float graphics::bloom::Exposure = 2.8f;

static Microsoft::WRL::ComPtr<ID3D11Buffer> g_pcbBloom = NULL;
static Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShaderBloom = NULL;

//Cutoff shaders
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloomCutoff = NULL;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_cutoffTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_cutoffTextureSRV = NULL;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_cutoffTextureRTV = NULL;

//Blur shaders
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloomBlur = NULL;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_horizontalBlurTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_horizontalBlurTextureSRV = NULL;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_horizontalBlurTextureRTV = NULL;

static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_completeBlurTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_completeBlurTextureSRV = NULL;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_completeBlurTextureRTV = NULL;

//Merge shaders
static Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShaderBloomMerge = NULL;
static Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pSamAni = NULL;

//Input texture
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_pProxyTexture = NULL;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> g_pProxyTextureSRV = NULL;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> graphics::bloom::g_inputRenderTarget = NULL;

namespace graphics
{
	struct CV_Bloom
	{
		uint32_t horizontal;
		float exposure;
		float pad1;
		float pad2;
	};

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

		//Horizontal bluut texture
		auto hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_cutoffTexture.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_cutoffTexture.Get(), 0, g_horizontalBlurTextureRTV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_cutoffTexture.Get(), 0, g_horizontalBlurTextureSRV.GetAddressOf());
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

		auto hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_horizontalBlurTexture.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_horizontalBlurTexture.Get(), 0, g_cutoffTextureRTV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_horizontalBlurTexture.Get(), 0, g_cutoffTextureSRV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);

		hr = graphics::g_d3dDevice->CreateTexture2D(&desc, 0, g_completeBlurTexture.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateRenderTargetView(g_completeBlurTexture.Get(), 0, g_completeBlurTextureRTV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
		hr = graphics::g_d3dDevice->CreateShaderResourceView(g_completeBlurTexture.Get(), 0, g_completeBlurTextureSRV.GetAddressOf());
		ASSERT_SUCCEEDED(hr);
	}

	void CreateBloomConstBuffer()
	{
		// Create constant buffer
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		cbDesc.ByteWidth = sizeof(CV_Bloom);
		ASSERT_SUCCEEDED(graphics::g_d3dDevice->CreateBuffer(&cbDesc, NULL, g_pcbBloom.GetAddressOf()), "fail to create Bloom const buffer.");
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

		//Create cutoff pixel shader.
		hr = graphics::g_d3dDevice->CreatePixelShader(g_pBloomCutOff_PS, sizeof(g_pBloomCutOff_PS), 0, g_pPixelShaderBloomCutoff.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

		//Create blur pixel shader.
		hr = graphics::g_d3dDevice->CreatePixelShader(g_pBloomBlur_PS, sizeof(g_pBloomBlur_PS), 0, g_pPixelShaderBloomBlur.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

		//Create merge pixel shader.
		hr = graphics::g_d3dDevice->CreatePixelShader(g_pBloomMerge_PS, sizeof(g_pBloomMerge_PS), 0, g_pPixelShaderBloomMerge.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

		CreateBloomConstBuffer();
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
		g_pProxyTexture.Reset();
		g_inputRenderTarget.Reset();
		g_pProxyTextureSRV.Reset();

		g_pPixelShaderBloomCutoff.Reset();
		g_cutoffTexture.Reset();
		g_cutoffTextureSRV.Reset();
		g_cutoffTextureRTV.Reset();

		g_pPixelShaderBloomBlur.Reset();
		g_horizontalBlurTexture.Reset();
		g_horizontalBlurTextureSRV.Reset();
		g_horizontalBlurTextureRTV.Reset();
	}

	void RenderCutOff(ID3D11RenderTargetView* const* target)
	{
		graphics::g_d3dImmediateContext->OMSetRenderTargets(1, g_cutoffTextureRTV.GetAddressOf(), nullptr);
		graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderBloomCutoff.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_pProxyTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->Draw(4, 0);
	}

	void RenderBlur(ID3D11RenderTargetView* const* target)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;

		auto hr = (graphics::g_d3dImmediateContext->Map(g_pcbBloom.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		ASSERT_SUCCEEDED(hr, "Fail to map FXAA resource");

		CV_Bloom* pBloom = (CV_Bloom*)MappedResource.pData;
		pBloom->horizontal = 1;
		pBloom->exposure = bloom::Exposure;
		graphics::g_d3dImmediateContext->Unmap(g_pcbBloom.Get(), 0);

		//Draw horizontal blur.
		graphics::g_d3dImmediateContext->OMSetRenderTargets(1, g_horizontalBlurTextureRTV.GetAddressOf(), nullptr);
		graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderBloomBlur.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetConstantBuffers(0, 1, g_pcbBloom.GetAddressOf());
		graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_cutoffTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->Draw(4, 0);

		hr = (graphics::g_d3dImmediateContext->Map(g_pcbBloom.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		ASSERT_SUCCEEDED(hr, "Fail to map FXAA resource");
		pBloom = (CV_Bloom*)MappedResource.pData;
		pBloom->horizontal = 0;
		pBloom->exposure = bloom::Exposure;
		graphics::g_d3dImmediateContext->Unmap(g_pcbBloom.Get(), 0);

		////Draw complete blur.
		graphics::g_d3dImmediateContext->OMSetRenderTargets(1, g_completeBlurTextureRTV.GetAddressOf(), nullptr);
		graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderBloomBlur.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetConstantBuffers(0, 1, g_pcbBloom.GetAddressOf());
		graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_horizontalBlurTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->Draw(4, 0);
	}

	void MergeBlurToScene(ID3D11RenderTargetView* const* target)
	{
		graphics::g_d3dImmediateContext->OMSetRenderTargets(1, target, nullptr);
		graphics::g_d3dImmediateContext->PSSetConstantBuffers(0, 1, g_pcbBloom.GetAddressOf());
		graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		graphics::g_d3dImmediateContext->VSSetShader(g_pVertexShaderBloom.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShader(g_pPixelShaderBloomMerge.Get(), NULL, 0);
		graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, g_pProxyTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->PSSetShaderResources(1, 1, g_completeBlurTextureSRV.GetAddressOf());
		graphics::g_d3dImmediateContext->Draw(4, 0);
	}

	void bloom::Render(ID3D11RenderTargetView* const* target)
	{
		RenderCutOff(target);
		RenderBlur(target);
		MergeBlurToScene(target);
	}
}