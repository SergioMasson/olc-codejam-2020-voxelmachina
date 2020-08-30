#include "pch.h"
#include "voxelMachina.h"
#include "coreGraphics.h"
#include "input.h"
#include "colors.h"
#include "camera.h"
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

struct ObjectConstBuffer
{
	DirectX::XMFLOAT4X4 ModelMatrix;
};

struct SceneConstBuffer
{
	DirectX::XMFLOAT4X4 ViewProjectMatrix;
};

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

template<class T>
void BuildConstantBuffer(_Out_ ID3D11Buffer** constantBuffer)
{
	// constant colour buffer. TODO(Replace this with all camera and light buffers.)
	D3D11_BUFFER_DESC bd = { 0 };
	bd.ByteWidth = sizeof(T);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	auto hr = graphics::g_d3dDevice->CreateBuffer(&bd, nullptr, constantBuffer);
}

void VoxelMachinaApp::Startup(void)
{
	BuildShadersAndInputLayout(m_vertexShader.GetAddressOf(), m_pixelShader.GetAddressOf(), m_inputLayout.GetAddressOf());
	BuildIndexAndVertexBuffer(m_vertexBuffer.GetAddressOf(), m_indexBuffer.GetAddressOf());

	//Build model const buffer.
	BuildConstantBuffer<ObjectConstBuffer>(m_ObjectConstBuffer.GetAddressOf());
	BuildConstantBuffer<SceneConstBuffer>(m_sceneConstBuffer.GetAddressOf());

	//Build scene const buffer.

	//Initialize all matrix.
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_WorldMatrix, I);

	math::Vector3 cameraPosition{ 0.0f, 0.0f, -6.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };

	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);

	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
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
		m_sceneCamera.SetPosition(m_sceneCamera.GetPosition() + (math::Vector3{ 0, 0, 60 } *deltaT));
	else if (Input::IsPressed(Input::KeyCode::Key_s))
		m_sceneCamera.SetPosition(m_sceneCamera.GetPosition() - (math::Vector3{ 0, 0, 60 } *deltaT));

	if (Input::IsPressed(Input::KeyCode::Key_a))
		m_sceneCamera.SetPosition(m_sceneCamera.GetPosition() + (math::Vector3{ 60, 0, 0 } *deltaT));
	else if (Input::IsPressed(Input::KeyCode::Key_d))
		m_sceneCamera.SetPosition(m_sceneCamera.GetPosition() - (math::Vector3{ 60, 0, 0 } *deltaT));
}

void VoxelMachinaApp::RenderScene(void)
{
	//Sets vertex shader input layout.
	graphics::g_d3dImmediateContext->IASetInputLayout(m_inputLayout.Get());

	//Sets primitive topology.
	graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_WorldMatrix);
	XMFLOAT4X4 objectWorldMatrix;

	//Update object world matrix.
	XMStoreFloat4x4(&objectWorldMatrix, world);
	graphics::g_d3dImmediateContext->UpdateSubresource(m_ObjectConstBuffer.Get(), 0, 0, &objectWorldMatrix, 0, 0);

	XMFLOAT4X4 viewProjectionMatrix;
	m_sceneCamera.StoreViewProjectionMatrix(&viewProjectionMatrix);

	graphics::g_d3dImmediateContext->UpdateSubresource(m_sceneConstBuffer.Get(), 0, 0, &viewProjectionMatrix, 0, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//Bind shaders to graphics context.
	graphics::g_d3dImmediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	//Bind model matrix to slot b0
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(0, 1, m_ObjectConstBuffer.GetAddressOf());

	//Bind view projection matrix to slot b1
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(1, 1, m_sceneConstBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//Binds vertex buffer and index buffer.
	graphics::g_d3dImmediateContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	graphics::g_d3dImmediateContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	graphics::g_d3dImmediateContext->DrawIndexed(36, 0, 0);
}

void VoxelMachinaApp::RenderUI(void)
{
}