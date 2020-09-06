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
#include <string>

void LightVoxelMachinaApp::Startup(void)
{
	//Create the render pipeline
	m_renderPipeline = std::make_unique<graphics::RenderPipeline>();
	m_renderPipeline->LoadShader(g_pSimpleLightVS, sizeof(g_pSimpleLightVS), g_pSimpleLightPS, sizeof(g_pSimpleLightPS));

	m_skybox = new graphics::Texture2D(L"textures/DarkSkyCartoon.dds");
	m_renderPipeline->SetSkyboxTexture(m_skybox);

	CreateLights();
	CreateObjects();
	CreateGUI();
	CreateCamera();

	m_playerController = new PlayerController(math::Vector3(0, 1, 0), m_player, &m_sceneCamera);
	m_cameraController = new CameraController(m_sceneCamera, math::Vector3(0, 1, 0));
}

bool LightVoxelMachinaApp::IsDone()
{
	return m_isDone;
}

void LightVoxelMachinaApp::Update(float deltaT)
{
	if (m_firstFrame)
	{
		m_firstFrame = false;
		return;
	}

	m_isDone = Input::IsPressed(Input::KeyCode::Key_escape);
	m_playerController->Update(deltaT);
	m_cameraController->Update(deltaT);

	math::Vector3 lightPosition = m_player->GetPosition() + (m_player->GetRotation() * math::Vector3{ 0, 1, 1 });
	math::Vector3 playerFoward = math::Matrix3{ m_player->GetRotation() }.GetZ();

	DirectX::XMStoreFloat3(&m_scenePointLight.Position, lightPosition);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Position, lightPosition);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Direction, playerFoward);

	m_time -= deltaT;
	m_counterText->SetText(L"Time left: " + std::to_wstring(m_time));
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

	//Draw all GUI elements.
	for (auto guiElement : m_sceneGuiElements)
		guiElement->Draw();

	ASSERT_SUCCEEDED(graphics::g_d2dDeviceContext->EndDraw());
}

void LightVoxelMachinaApp::Resize(uint32_t width, uint32_t height)
{
	auto aspectRation = static_cast<float>(width) / static_cast<float>(height);
	//m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	//m_sceneCamera.Update();
}

void LightVoxelMachinaApp::CreateCamera()
{
	m_sceneCamera = Camera();
	math::Vector3 cameraPosition{ 0.0f, 2.0f, 10.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };
	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	m_sceneCamera.Update();
}

void LightVoxelMachinaApp::CreateLights()
{
	m_sceneDirLight = DirectionalLight{};
	//Directional light.
	m_sceneDirLight.Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_sceneDirLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneDirLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneDirLight.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	m_sceneDirLight.Intensity = 0;

	m_scenePointLight = PointLight{};
	// Point light--position is changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_scenePointLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_scenePointLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_scenePointLight.Att = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);
	m_scenePointLight.Range = 5;
	m_scenePointLight.Position = DirectX::XMFLOAT3(0.0f, 3.0f, 5.0f);
	m_scenePointLight.Intensity = 0;

	m_sceneSpotLight = SpotLight{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_sceneSpotLight.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
	m_sceneSpotLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Att = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 5.0;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_sceneSpotLight.Intensity = 2;
}

void LightVoxelMachinaApp::CreateObjects()
{
	graphics::MeshData quad;
	graphics::MeshData::CreateGrid(100, 100, 20, 20, quad);

	graphics::MeshData playerCharacter;
	graphics::MeshData::LoadFromOBJFile(L"models/littleRobot.model", playerCharacter);

	graphics::MeshData enemyMesh;
	graphics::MeshData::LoadFromOBJFile(L"models/enemy.model", enemyMesh);

	//TODO(Sergio): Load this on a separate thread. Asset loading will take a time...
	auto normalMap = new graphics::Texture2D(L"textures/neutralNormal.dds");
	auto playerTexture = new graphics::Texture2D(L"textures/littleRobot.dds");
	auto floorTexture = new graphics::Texture2D(L"textures/checkboard_mips.dds");
	auto floorNormalMap = new graphics::Texture2D(L"textures/tile_nmap.dds");
	auto enemyTexture = new graphics::Texture2D(L"textures/enemy.dds");

	graphics::Material material1{};
	material1.Ambient = colors::white;
	material1.Diffuse = colors::white;
	material1.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2.0f);

	m_player = new graphics::MeshRenderer(playerCharacter, material1, math::Vector3(0, 0, 0), math::Quaternion(), math::Vector3(1, 1, 1));
	m_player->SetAlbedoTexture(playerTexture);
	m_player->SetNormalMap(normalMap);
	m_sceneMeshRenderer.push_back(m_player);

	auto enemy = new graphics::MeshRenderer(enemyMesh, material1, math::Vector3(2, 0, 0), math::Quaternion());
	enemy->SetAlbedoTexture(enemyTexture);
	enemy->SetNormalMap(normalMap);
	m_sceneMeshRenderer.push_back(enemy);

	graphics::Material material2{};
	material2.Ambient = colors::silver;
	material2.Diffuse = colors::silver;
	material2.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2);

	auto floor = new graphics::MeshRenderer(quad, material2, math::Vector3(0, 0, 0), math::Quaternion());
	floor->SetAlbedoTexture(floorTexture);
	floor->SetNormalMap(floorNormalMap);
	floor->SetTextureScale(10, 10);

	m_sceneMeshRenderer.push_back(floor);
}

void LightVoxelMachinaApp::CreateGUI()
{
	m_informationText = new graphics::UI::GuiText(nullptr, 0, 0, 500, 100, 30);

	m_informationText->SetText(L"5 Evil robots are lost in the room, find all of them before the time is over.");
	m_informationText->SetColor(D2D1::ColorF::White);

	m_sceneGuiElements.push_back(m_informationText);

	m_counterText = new graphics::UI::GuiText(nullptr, -500, 0, 500, 100, 20);
	m_counterText->SetColor(D2D1::ColorF::Yellow);
	m_counterText->SetAnchorType(graphics::UI::ParentAnchorType::TopRight);
	m_counterText->SetText(L"Time left: 30");

	m_sceneGuiElements.push_back(m_counterText);
}

void LightVoxelMachinaApp::Cleanup(void)
{
}