#include "pch.h"
#include "renderPipeline.h"
#include "coreGraphics.h"
#include "graphicsUtils.h"

struct SceneConstBuffer
{
	SceneConstBuffer() { ZeroMemory(this, sizeof(this)); }

	DirectionalLight DirectionalLights;
	PointLight PointLight;
	SpotLight SpotLight;

	//Position of the camera in world space.
	DirectX::XMFLOAT3 eyeWorld;
	float pad;
};

struct ObjectConstBuffer
{
	ObjectConstBuffer() { ZeroMemory(this, sizeof(this)); }
	DirectX::XMFLOAT4X4 ObjectModelMatrix;
	DirectX::XMFLOAT4X4 WorldInvTranspose;
	DirectX::XMFLOAT4X4 ViewProjectionMatrix;
	graphics::Material ObjectMaterial;
};

graphics::RenderPipeline::RenderPipeline()
{
	BuildConstantBuffer<SceneConstBuffer>(m_sceneConstBuffer.GetAddressOf());
	BuildConstantBuffer<ObjectConstBuffer>(m_objectConstBuffer.GetAddressOf());
}

graphics::RenderPipeline::~RenderPipeline()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_inputLayout = nullptr;
	m_sceneConstBuffer = nullptr;
	m_objectConstBuffer = nullptr;
}

void graphics::RenderPipeline::LoadShader(const BYTE* vertexShader, SIZE_T vertexShaderSize, const BYTE* pixelShader, SIZE_T pixelShaderSize)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	//Create shaders.
	//Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[4] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	auto hr = graphics::g_d3dDevice->CreateVertexShader(vertexShader, vertexShaderSize, 0, m_vertexShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create vertex shader.");

	hr = graphics::g_d3dDevice->CreatePixelShader(pixelShader, pixelShaderSize, 0, m_pixelShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

	hr = graphics::g_d3dDevice->CreateInputLayout(vertexDesc, 4, vertexShader, vertexShaderSize, m_inputLayout.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create input layout");
}

void graphics::RenderPipeline::StartRender(Camera* camera, DirectionalLight lights, SpotLight spotLights, PointLight pointLights)
{
	m_camera = camera;

	//Sets vertex shader input layout.
	graphics::g_d3dImmediateContext->IASetInputLayout(m_inputLayout.Get());

	//Sets primitive topology.
	graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Bind shaders to graphics context.
	graphics::g_d3dImmediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	graphics::g_d3dImmediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	SceneConstBuffer sceneBuffer{};

	DirectX::XMStoreFloat3(&sceneBuffer.eyeWorld, camera->GetPosition());

	sceneBuffer.DirectionalLights = lights;
	sceneBuffer.SpotLight = spotLights;
	sceneBuffer.PointLight = pointLights;

	//Updates the subresource.
	graphics::g_d3dImmediateContext->UpdateSubresource(m_sceneConstBuffer.Get(), 0, 0, &sceneBuffer, 0, 0);

	//Bind view projection matrix to slot b1
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(1, 1, m_sceneConstBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetConstantBuffers(1, 1, m_sceneConstBuffer.GetAddressOf());
}

void graphics::RenderPipeline::RenderMesh(MeshRenderer const& mesh)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ObjectConstBuffer objectBuffer{};

	math::Matrix4 mvpMatrix = (mesh.m_worldMatrix.Transpose() * m_camera->GetViewProjectionMatrix());
	math::Matrix4 worldInvTranspose = mesh.m_worldMatrix.Transpose().InverseTranspose();

	DirectX::XMStoreFloat4x4(&objectBuffer.ObjectModelMatrix, mesh.m_worldMatrix);
	DirectX::XMStoreFloat4x4(&objectBuffer.ViewProjectionMatrix, mvpMatrix);
	DirectX::XMStoreFloat4x4(&objectBuffer.WorldInvTranspose, worldInvTranspose);

	objectBuffer.ObjectMaterial = mesh.m_material;

	graphics::g_d3dImmediateContext->UpdateSubresource(m_objectConstBuffer.Get(), 0, 0, &objectBuffer, 0, 0);

	//Bind view projection matrix to slot b0
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(0, 1, m_objectConstBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetConstantBuffers(0, 1, m_objectConstBuffer.GetAddressOf());

	//Binds vertex buffer and index buffer.
	graphics::g_d3dImmediateContext->IASetVertexBuffers(0, 1, mesh.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	graphics::g_d3dImmediateContext->IASetIndexBuffer(mesh.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	graphics::g_d3dImmediateContext->DrawIndexed(static_cast<UINT>(mesh.m_meshData.Indices.size()), 0, 0);
}