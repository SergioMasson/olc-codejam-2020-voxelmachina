#include "pch.h"
#include "renderPipeline.h"
#include "coreGraphics.h"
#include "graphicsUtils.h"
#include "SkyboxVS.h"
#include "SkyboxPS.h"

struct SkyboxConstBuffer
{
	DirectX::XMFLOAT4X4 MVP;
};

struct SceneConstBuffer
{
	SceneConstBuffer() { ZeroMemory(this, sizeof(this)); }

	Light PointLight;
	Light DirectionalLights;
	Light SpotLight;

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
	graphics::Material	ObjectMaterial;
	DirectX::XMFLOAT2	TextureScale;
	DirectX::XMFLOAT2	TextureDisplacement;
};

graphics::RenderPipeline::RenderPipeline()
{
	BuildConstantBuffer<SceneConstBuffer>(m_sceneConstBuffer.GetAddressOf());
	BuildConstantBuffer<ObjectConstBuffer>(m_objectConstBuffer.GetAddressOf());
	BuildConstantBuffer<SkyboxConstBuffer>(m_skyboxConstBuffer.GetAddressOf());
	CreateSkybox();
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
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	auto hr = graphics::g_d3dDevice->CreateVertexShader(vertexShader, vertexShaderSize, 0, m_vertexShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create vertex shader.");

	hr = graphics::g_d3dDevice->CreatePixelShader(pixelShader, pixelShaderSize, 0, m_pixelShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create pixel shader.");

	hr = graphics::g_d3dDevice->CreateInputLayout(vertexDesc, 4, vertexShader, vertexShaderSize, m_inputLayout.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create input layout");
}

void graphics::RenderPipeline::StartRender(Camera* camera, Light lights, Light spotLights, Light pointLights)
{
	m_camera = camera;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	graphics::g_d3dImmediateContext->RSGetState(rasterizerState.GetAddressOf());

	if (m_skyboxTexture != nullptr)
	{
		RenderSkybox(camera);
		graphics::g_d3dImmediateContext->RSSetState(rasterizerState.Get());
	}

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
	//If mesh bouding sphere is not inside camera`s frustum skip this mesh.
	if (!m_camera->GetWorldFrustum().IntersectSphere(mesh.GetWorldBoudingSphere()))
		return;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ObjectConstBuffer objectBuffer{};

	math::Matrix4 mvpMatrix = (mesh.m_worldMatrix.Transpose() * m_camera->GetViewProjectionMatrix());
	math::Matrix4 worldInvTranspose = mesh.m_worldMatrix.Transpose().InverseTranspose();
	math::Matrix4 world = mesh.m_worldMatrix.Transpose();

	DirectX::XMStoreFloat4x4(&objectBuffer.ObjectModelMatrix, world);
	DirectX::XMStoreFloat4x4(&objectBuffer.ViewProjectionMatrix, mvpMatrix);
	DirectX::XMStoreFloat4x4(&objectBuffer.WorldInvTranspose, worldInvTranspose);

	objectBuffer.ObjectMaterial = mesh.m_material;
	objectBuffer.TextureScale = mesh.m_textureScale;
	objectBuffer.TextureDisplacement = mesh.m_textureDisplacement;

	graphics::g_d3dImmediateContext->UpdateSubresource(m_objectConstBuffer.Get(), 0, 0, &objectBuffer, 0, 0);

	//Bind view projection matrix to slot b0
	graphics::g_d3dImmediateContext->VSSetConstantBuffers(0, 1, m_objectConstBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetConstantBuffers(0, 1, m_objectConstBuffer.GetAddressOf());

	graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, mesh.m_albedoTexture->m_resourceView.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetShaderResources(1, 1, mesh.m_normalMap->m_resourceView.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetShaderResources(2, 1, mesh.m_emissionTexture->m_resourceView.GetAddressOf());

	graphics::g_d3dImmediateContext->PSSetSamplers(0, 1, mesh.m_albedoTexture->m_samplerState.GetAddressOf());

	//Binds vertex buffer and index buffer.
	graphics::g_d3dImmediateContext->IASetVertexBuffers(0, 1, mesh.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	graphics::g_d3dImmediateContext->IASetIndexBuffer(mesh.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	graphics::g_d3dImmediateContext->DrawIndexed(static_cast<UINT>(mesh.m_meshData->Indices.size()), 0, 0);
}

void graphics::RenderPipeline::CreateSkybox()
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
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	auto hr = graphics::g_d3dDevice->CreateVertexShader(g_pSkyboxVS, sizeof(g_pSkyboxVS), 0, m_skyboxVertexShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create skybox vertex shader.");

	hr = graphics::g_d3dDevice->CreatePixelShader(g_pSkyboxPS, sizeof(g_pSkyboxPS), 0, m_skyboxPixelShader.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create skybox pixel shader.");

	hr = graphics::g_d3dDevice->CreateInputLayout(vertexDesc, 4, g_pSkyboxVS, sizeof(g_pSkyboxVS), m_skyboxInputLayout.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create skybox input layout");

	MeshData* skyboxMeshFull = new MeshData();
	MeshData::CreateBox(10000, 10000, 10000, *skyboxMeshFull);

	Material material;
	m_skyboxMeshRenerer = MeshRenderer(skyboxMeshFull, material, math::Vector3(), math::Quaternion());

	D3D11_RASTERIZER_DESC rasterizerStateDesc;
	ZeroMemory(&rasterizerStateDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
	rasterizerStateDesc.FrontCounterClockwise = TRUE;
	rasterizerStateDesc.DepthBias = 0;
	rasterizerStateDesc.DepthBiasClamp = 0.0f;
	rasterizerStateDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	rasterizerStateDesc.ScissorEnable = FALSE;
	rasterizerStateDesc.MultisampleEnable = FALSE;
	rasterizerStateDesc.AntialiasedLineEnable = FALSE;

	hr = graphics::g_d3dDevice->CreateRasterizerState(&rasterizerStateDesc, m_skyboxRasterizerState.GetAddressOf());
	ASSERT_SUCCEEDED(hr, "Fail to create skybox rasterizerState");

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	graphics::g_d3dDevice->CreateDepthStencilState(&dsDesc, m_skyboxDepthStencilState.GetAddressOf());
}

void graphics::RenderPipeline::RenderSkybox(Camera* camera)
{
	//Sets vertex shader input layout.
	graphics::g_d3dImmediateContext->IASetInputLayout(m_skyboxInputLayout.Get());

	//Sets primitive topology.
	graphics::g_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	graphics::g_d3dImmediateContext->RSSetState(m_skyboxRasterizerState.Get());
	graphics::g_d3dImmediateContext->OMSetDepthStencilState(m_skyboxDepthStencilState.Get(), 1);

	//Bind shaders to graphics context.
	graphics::g_d3dImmediateContext->VSSetShader(m_skyboxVertexShader.Get(), nullptr, 0);
	graphics::g_d3dImmediateContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);

	math::Matrix4 mvpMatrix = camera->GetViewProjectionMatrix();
	DirectX::XMFLOAT4X4 mvp;
	DirectX::XMStoreFloat4x4(&mvp, mvpMatrix);

	graphics::g_d3dImmediateContext->UpdateSubresource(m_skyboxConstBuffer.Get(), 0, 0, &mvp, 0, 0);

	graphics::g_d3dImmediateContext->VSSetConstantBuffers(0, 1, m_skyboxConstBuffer.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetShaderResources(0, 1, m_skyboxTexture->m_resourceView.GetAddressOf());
	graphics::g_d3dImmediateContext->PSSetSamplers(0, 1, m_skyboxTexture->m_samplerState.GetAddressOf());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//Binds vertex buffer and index buffer.
	graphics::g_d3dImmediateContext->IASetVertexBuffers(0, 1, m_skyboxMeshRenerer.m_vertexBuffer.GetAddressOf(), &stride, &offset);
	graphics::g_d3dImmediateContext->IASetIndexBuffer(m_skyboxMeshRenerer.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	graphics::g_d3dImmediateContext->DrawIndexed(static_cast<UINT>(m_skyboxMeshRenerer.m_meshData->Indices.size()), 0, 0);
}