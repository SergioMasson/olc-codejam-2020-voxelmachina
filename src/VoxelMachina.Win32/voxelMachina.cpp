#include "pch.h"
#include "voxelMachina.h"
#include "coreGraphics.h"
#include "input.h"
#include "colors.h"
#include "math.h"
#include "BoxPS.h"
#include "BoxVS.h"

using namespace Microsoft::WRL;
using namespace DirectX;

static bool g_isDone = false;

//Vertex data structure. TODO: Upgrade this with normal, tangent and texture coordinate.
struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

math::Vector3 cameraPosition{ 0.0f, 0.0f, -6.0f };

void BuildShadersAndInputLayout(_Out_ ID3D11VertexShader** vertexShader, _Out_ ID3D11PixelShader** pixelShader, _Out_ ID3D11InputLayout** inputLayout)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	//Create shaders.

	auto hr = graphics::g_d3dDevice->CreateVertexShader(g_pBoxVS, sizeof(g_pBoxVS), 0, vertexShader);
	ASSERT_SUCCEEDED(hr, "Fail to create vertex shader.");

	hr = graphics::g_d3dDevice->CreatePixelShader(g_pBoxPS, sizeof(g_pBoxPS), 0, pixelShader);
	ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = graphics::g_d3dDevice->CreateInputLayout(vertexDesc, 2, g_pBoxVS, sizeof(g_pBoxVS), inputLayout);
	ASSERT_SUCCEEDED(hr, "Fail to create input layout");
}

void BuildIndexAndVertexBuffer(_Out_ ID3D11Buffer** vertexBuffer, _Out_ ID3D11Buffer** indexBuffer)
{
	// Create vertex buffer
	Vertex vertices[] =
	{
		{ Vertex{ XMFLOAT3(-1.0f, -1.0f, -1.0f), colors::white }   },
		{ Vertex{ XMFLOAT3(-1.0f, +1.0f, -1.0f), colors::black }   },
		{ Vertex{ XMFLOAT3(+1.0f, +1.0f, -1.0f), colors::red   }   },
		{ Vertex{ XMFLOAT3(+1.0f, -1.0f, -1.0f), colors::green }   },
		{ Vertex{ XMFLOAT3(-1.0f, -1.0f, +1.0f), colors::blue  }   },
		{ Vertex{ XMFLOAT3(-1.0f, +1.0f, +1.0f), colors::yellow}   },
		{ Vertex{ XMFLOAT3(+1.0f, +1.0f, +1.0f), colors::cyan  }   },
		{ Vertex{ XMFLOAT3(+1.0f, -1.0f, +1.0f), colors::magenta } }
	};

	D3D11_BUFFER_DESC vbd;

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;

	auto hr = graphics::g_d3dDevice->CreateBuffer(&vbd, &vinitData, vertexBuffer);

	ASSERT_SUCCEEDED(hr, "Fail to create vertex buffer.");

	//Cubes index buffer.
	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	hr = graphics::g_d3dDevice->CreateBuffer(&ibd, &iinitData, indexBuffer);
}

void BuildConstantBuffer(_Out_ ID3D11Buffer** constantBuffer)
{
	// constant colour buffer. TODO(Replace this with all camera and light buffers.)
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(XMFLOAT4X4);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	auto hr = graphics::g_d3dDevice->CreateBuffer(&bd, nullptr, constantBuffer);
}

void VoxelMachinaApp::Startup(void)
{
	BuildShadersAndInputLayout(m_vertexShader.GetAddressOf(), m_pixelShader.GetAddressOf(), m_inputLayout.GetAddressOf());
	BuildIndexAndVertexBuffer(m_vertexBuffer.GetAddressOf(), m_indexBuffer.GetAddressOf());
	BuildConstantBuffer(m_constBuffer.GetAddressOf());

	//Initialize all matrix.
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_WorldMatrix, I);
	XMStoreFloat4x4(&m_ViewMatrix, I);
	XMStoreFloat4x4(&m_ProjMatrix, I);

	// Build the view matrix.
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(cameraPosition, target, up);
	XMStoreFloat4x4(&m_ViewMatrix, V);

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_ProjMatrix, P);
}

void VoxelMachinaApp::Cleanup(void)
{
}

bool VoxelMachinaApp::IsDone(void)
{
	return g_isDone;
}

void VoxelMachinaApp::Update(float deltaT)
{
	g_isDone = Input::IsPressed(Input::KeyCode::Key_escape);

	if (Input::IsPressed(Input::KeyCode::Key_w))
		cameraPosition += (math::Vector3{ 0, 0, 60 } *deltaT);
	else if (Input::IsPressed(Input::KeyCode::Key_s))
		cameraPosition += (math::Vector3{ 0, 0, -60 } *deltaT);

	if (Input::IsPressed(Input::KeyCode::Key_a))
		cameraPosition += (math::Vector3{ 60, 0, 0 } *deltaT);
	else if (Input::IsPressed(Input::KeyCode::Key_d))
		cameraPosition += (math::Vector3{ -60, 0, 0 } *deltaT);

	// Update the view matrix.
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(cameraPosition, target, up);
	XMStoreFloat4x4(&m_ViewMatrix, V);
}

void VoxelMachinaApp::RenderScene(void)
{
	//Sets vertex shader input layout.
	graphics::g_d3dImmediateContext->IASetInputLayout(m_inputLayout.Get());

	//Sets primitive topology.
	graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_WorldMatrix);
	XMMATRIX view = XMLoadFloat4x4(&m_ViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&m_ProjMatrix);
	XMMATRIX worldViewProj = world * view * proj;
	XMFLOAT4X4 vertexShaderMatrix;

	XMStoreFloat4x4(&vertexShaderMatrix, worldViewProj);

	graphics::g_d3dImmediateContext->UpdateSubresource(m_constBuffer.Get(), 0, 0, &vertexShaderMatrix, 0, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//Bind shaders to graphics context.
	graphics::g_d3dImmediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//Binds vertex buffer and index buffer.
	graphics::g_d3dImmediateContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	graphics::g_d3dImmediateContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	graphics::g_d3dImmediateContext->DrawIndexed(36, 0, 0);
}

void VoxelMachinaApp::RenderUI(void)
{
}