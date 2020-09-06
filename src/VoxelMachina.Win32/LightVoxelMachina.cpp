#include "pch.h"
#include "LightVoxelMachina.h"
#include "application.h"
#include "graphics/meshRenderer.h"
#include "graphics/renderPipeline.h"
#include "graphics/coreGraphics.h"
#include "graphics/texture2D.h"
#include "input.h"
#include "colors.h"

#include "SimpleLightPS.h"
#include "SimpleLightVS.h"

void LightVoxelMachinaApp::Startup(void)
{
	//Create the render pipeline
	m_renderPipeline = std::make_unique<graphics::RenderPipeline>();
	m_renderPipeline->LoadShader(g_pSimpleLightVS, sizeof(g_pSimpleLightVS), g_pSimpleLightPS, sizeof(g_pSimpleLightPS));
	CreateCamera();

	m_skybox = new graphics::Texture2D(L"textures/DarkSkyCartoon.dds");
	m_renderPipeline->SetSkyboxTexture(m_skybox);

	CreateLights();
	CreateObjects();
	CreateGUI();
}

bool LightVoxelMachinaApp::IsDone()
{
	return m_isDone;
}

void LightVoxelMachinaApp::Update(float deltaT)
{
	m_isDone = Input::IsPressed(Input::KeyCode::Key_escape);

	m_cameraController->Update(deltaT);
}

void LightVoxelMachinaApp::RenderScene(void)
{
	m_renderPipeline->StartRender(&m_sceneCamera, m_sceneDirLight, m_sceneSpotLight, m_scenePointLight);

	for (auto renderer : m_sceneMeshRenderer)
		m_renderPipeline->RenderMesh(*renderer);
}

//TODO(Sergio): Implement those things later.
void LightVoxelMachinaApp::RenderUI(void)
{
	graphics::g_d2dDeviceContext->BeginDraw();

	ASSERT_SUCCEEDED(graphics::g_d2dDeviceContext->EndDraw());
}

void LightVoxelMachinaApp::Resize(uint32_t width, uint32_t height)
{
	auto aspectRation = static_cast<float>(width) / static_cast<float>(height);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	m_sceneCamera.Update();
}

void LightVoxelMachinaApp::CreateCamera()
{
	m_sceneCamera = Camera();
	math::Vector3 cameraPosition{ 10.0f, 10.0f, -10.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };
	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);
	//m_sceneCamera.SetPosition(cameraPosition);
	//m_sceneCamera.SetRotation(math::Quaternion());

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	m_sceneCamera.Update();

	m_cameraController = new CameraController(m_sceneCamera, math::Vector3(0, 1, 0));
}

void LightVoxelMachinaApp::CreateLights()
{
	m_sceneDirLight = DirectionalLight{};
	//Directional light.
	m_sceneDirLight.Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_sceneDirLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneDirLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneDirLight.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	m_sceneDirLight.Intensity = 1;

	m_scenePointLight = PointLight{};
	// Point light--position is changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_scenePointLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_scenePointLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_scenePointLight.Att = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);
	m_scenePointLight.Range = 25;
	m_scenePointLight.Position = DirectX::XMFLOAT3(6.0f, 3.0f, 3.0f);
	m_scenePointLight.Intensity = 1;

	m_sceneSpotLight = SpotLight{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_sceneSpotLight.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
	m_sceneSpotLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Att = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 25.0;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_sceneSpotLight.Intensity = 1;
}

void LightVoxelMachinaApp::CreateObjects()
{
	graphics::MeshData quad;
	graphics::MeshData::CreateGrid(100, 100, 20, 20, quad);

	graphics::MeshData playerCharacter;
	graphics::MeshData::LoadFromOBJFile(L"models/player.model", playerCharacter);

	//TODO(Sergio): Load this on a separate thread. Asset loading will take a time...
	auto normalMap = new graphics::Texture2D(L"textures/neutralNormal.dds");
	auto playerTexture = new graphics::Texture2D(L"textures/character.dds");
	auto floorTexture = new graphics::Texture2D(L"textures/checkboard_mips.dds");

	graphics::Material material1{};
	material1.Ambient = colors::white;
	material1.Diffuse = colors::white;
	material1.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2.0f);

	m_player = new graphics::MeshRenderer(playerCharacter, material1, math::Vector3(0, 0, 0), math::Quaternion(), math::Vector3(1, 1, 1));
	m_player->SetAlbedoTexture(playerTexture);
	m_player->SetNormalMap(normalMap);
	m_sceneMeshRenderer.push_back(m_player);

	graphics::Material material2{};
	material2.Ambient = colors::silver;
	material2.Diffuse = colors::silver;
	material2.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2);

	auto floor = new graphics::MeshRenderer(quad, material2, math::Vector3(0, 0, 0), math::Quaternion());
	floor->SetAlbedoTexture(floorTexture);
	floor->SetNormalMap(normalMap);
	floor->SetTextureScale(100, 100);

	m_sceneMeshRenderer.push_back(floor);
}

void LightVoxelMachinaApp::CreateGUI()
{
}

void LightVoxelMachinaApp::Cleanup(void)
{
}