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

#define ENEMY_COUNT 5
#define PILAR_COUNT 100
#define WORLD_X  100.0f
#define WORLD_Y  100.0f

void LightVoxelMachinaApp::Startup(void)
{
	enemiesLeft = ENEMY_COUNT;

	//Create the render pipeline
	m_renderPipeline = std::make_unique<graphics::RenderPipeline>();
	m_renderPipeline->LoadShader(g_pSimpleLightVS, sizeof(g_pSimpleLightVS), g_pSimpleLightPS, sizeof(g_pSimpleLightPS));

	/*m_skybox = new graphics::Texture2D(L"textures/DarkSkyCartoon.dds");
	m_renderPipeline->SetSkyboxTexture(m_skybox);*/

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

	m_isDone = Input::IsPressed(Input::KeyCode::Key_escape) || (Input::IsPressed(Input::KeyCode::Key_return) && enemiesLeft == 0);
	m_playerController->Update(deltaT);
	//m_cameraController->Update(deltaT);

	CheckForEnemyCollision();

	math::Vector3 lightPosition = m_player->GetPosition() + (m_player->GetRotation() * math::Vector3{ 0, 2, 0.25f });
	math::Vector3 playerFoward = math::Matrix3{ m_player->GetRotation() }.GetZ();

	DirectX::XMStoreFloat3(&m_scenePointLight.Position, lightPosition);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Position, lightPosition);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Direction, playerFoward);

	m_time += deltaT;
	m_counterText->SetText(L"TOTAL TIME: " + std::to_wstring(m_time));

	m_enemiesLeftText->SetText(L" X " + std::to_wstring(enemiesLeft));
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
	m_sceneDirLight = Light{};
	//Directional light.
	m_sceneDirLight.Ambient = 0.0f;
	m_sceneDirLight.Color = Color::White;
	m_sceneDirLight.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	m_sceneDirLight.Intensity = 0.0;

	m_scenePointLight = Light{};
	// Point light--position is changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = 0.2f;
	m_scenePointLight.Color = Color::White;
	m_scenePointLight.Range = 10.0f;
	m_scenePointLight.Position = DirectX::XMFLOAT3(0.0f, 3.0f, 5.0f);
	m_scenePointLight.Intensity = 0.5f;

	m_sceneSpotLight = Light{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = 1.0f;
	m_scenePointLight.Color = Color::White;
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 25.0f;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_sceneSpotLight.Intensity = 0.0f;
}

void LightVoxelMachinaApp::CreateObjects()
{
	graphics::MeshData quad;
	graphics::MeshData::CreateGrid(100, 100, 20, 20, quad);

	graphics::MeshData playerCharacter;
	graphics::MeshData::LoadFromOBJFile(L"models/littleRobot.model", playerCharacter);

	graphics::MeshData enemyMesh;
	graphics::MeshData::LoadFromOBJFile(L"models/enemy.model", enemyMesh);

	graphics::MeshData pilarMesh;
	graphics::MeshData::CreateCylinder(1, 1, 10, 20, 20, pilarMesh);

	//TODO(Sergio): Load this on a separate thread. Asset loading will take a time...
	auto normalMap = new graphics::Texture2D(L"textures/neutralNormal.dds");
	auto playerTexture = new graphics::Texture2D(L"textures/littleRobot.dds");
	auto floorTexture = new graphics::Texture2D(L"textures/checkboard_mips.dds");
	auto floorNormalMap = new graphics::Texture2D(L"textures/tile_nmap.dds");
	auto enemyTexture = new graphics::Texture2D(L"textures/enemy.dds");
	auto enemyDetectedTexture = new graphics::Texture2D(L"textures/enemy_inv.dds");
	auto pilarTexture = new graphics::Texture2D(L"textures/checkboard_mips.dds");
	auto pilarNormal = new graphics::Texture2D(L"textures/tile_nmap.dds");

	graphics::Material material1{};
	material1.Diffuse = 1.0f;
	material1.Specular = 2.0f;
	material1.Color = Color::White;

	m_player = new graphics::MeshRenderer(playerCharacter, material1, math::Vector3(0, 0, 0), math::Quaternion(), math::Vector3(1, 1, 1));
	m_player->SetAlbedoTexture(playerTexture);
	m_player->SetNormalMap(normalMap);
	m_sceneMeshRenderer.push_back(m_player);

	CreateEnemy(enemyMesh, enemyTexture, normalMap, enemyDetectedTexture);
	CreatePilars(pilarMesh, pilarTexture, pilarNormal);

	graphics::Material material2{};
	material2.Color = Color::Silver;
	material2.Specular = 2.0f;
	material2.Diffuse = 1.0f;

	auto floor = new graphics::MeshRenderer(quad, material2, math::Vector3(0, 0, 0), math::Quaternion());
	floor->SetAlbedoTexture(floorTexture);
	floor->SetNormalMap(floorNormalMap);
	floor->SetTextureScale(20, 20);

	m_sceneMeshRenderer.push_back(floor);
}

void LightVoxelMachinaApp::CreateEnemy(graphics::MeshData& enemyData, graphics::Texture2D* enemyTexture, graphics::Texture2D* enemyNormal, graphics::Texture2D* detectedTexture)
{
	graphics::Material material1{};
	material1.Color = Color::White;
	material1.Diffuse = 1.0f;
	material1.Specular = 2.0f;

	for (size_t i = 0; i < ENEMY_COUNT; i++)
	{
		float randomX = (rand() % 1000) / 1000.0f;
		float randomY = rand() % 1000 / 1000.0f;

		float enemyX = (randomX * WORLD_X) - ((WORLD_X) / 2.0f);
		float enemyY = (randomY * WORLD_Y) - ((WORLD_Y) / 2.0f);
		auto enemyPosition = math::Vector3(enemyX, 0, enemyY);
		auto enemy = new graphics::MeshRenderer(enemyData, material1, enemyPosition, math::Quaternion());
		enemy->SetAlbedoTexture(enemyTexture);
		enemy->SetNormalMap(enemyNormal);
		m_sceneMeshRenderer.push_back(enemy);
		m_enemiesLeft.push_back(new Enemy(enemy, detectedTexture));
	}
}

void LightVoxelMachinaApp::CreatePilars(graphics::MeshData& pilarData, graphics::Texture2D* pilarTexture, graphics::Texture2D* pilarNormal)
{
	graphics::Material material1{};
	material1.Color = Color::White;
	material1.Diffuse = 1.0f;
	material1.Specular = 2.0f;

	for (size_t i = 0; i < PILAR_COUNT; i++)
	{
		float pilarX = 0.0f;
		float pilarY = 0.0f;

		math::Vector3 distance;

		bool overlapingEnemy = false;

		do
		{
			float randomX = (rand() % 1000) / 1000.0f;
			float randomY = rand() % 1000 / 1000.0f;

			pilarX = (randomX * (WORLD_X - 3.0f)) - ((WORLD_X - 3.0f) / 2.0f);
			pilarY = (randomY * (WORLD_Y - 3.0f)) - ((WORLD_Y - 3.0f) / 2.0f);

			distance = m_player->GetPosition() - math::Vector3(pilarX, 0, pilarY);

			for (auto enemy : m_enemiesLeft)
			{
				distance = enemy->GetPosition() - math::Vector3(pilarX, 0, pilarY);

				if (math::Length(distance) <= 1.0f)
					overlapingEnemy = true;
			}
		} while (math::Length(distance) <= 10.0f && overlapingEnemy);

		//Check for player

		//Check for enemy

		auto pilar = new graphics::MeshRenderer(pilarData, material1, math::Vector3(pilarX, 5, pilarY), math::Quaternion());
		pilar->SetAlbedoTexture(pilarTexture);
		pilar->SetNormalMap(pilarNormal);
		pilar->SetTextureScale(3, 5);
		m_sceneMeshRenderer.push_back(pilar);
	}
}

void LightVoxelMachinaApp::CreateGUI()
{
	m_informationText = new graphics::UI::GuiText(nullptr, 10, 10, 500, 100, 30);

	m_informationText->SetText(std::to_wstring(enemiesLeft) + L" evil robots are lost in the room, find all of them! How fast can you do it?");
	m_informationText->SetColor(D2D1::ColorF::White);

	m_sceneGuiElements.push_back(m_informationText);

	m_counterText = new graphics::UI::GuiText(nullptr, -500, 0, 500, 100, 20);
	m_counterText->SetColor(D2D1::ColorF::Yellow);
	m_counterText->SetAnchorType(graphics::UI::ParentAnchorType::TopRight);
	m_counterText->SetText(L"Time left: 30");

	m_sceneGuiElements.push_back(m_counterText);

	auto scorePanel = new graphics::UI::GuiPanel(nullptr, -200, 30, 170, 100);
	scorePanel->SetColor(Color{ Color::White, 0.5f });
	scorePanel->SetAnchorType(graphics::UI::ParentAnchorType::TopRight);
	m_sceneGuiElements.push_back(scorePanel);

	m_enemiesLeftText = new graphics::UI::GuiText(scorePanel, 80, 0, 300, 100, 30);
	m_enemiesLeftText->SetColor(D2D1::ColorF::Purple);
	m_enemiesLeftText->SetAnchorType(graphics::UI::ParentAnchorType::TopLeft);
	m_enemiesLeftText->SetText(L" X " + std::to_wstring(enemiesLeft));

	m_sceneGuiElements.push_back(m_enemiesLeftText);

	auto enemy_icon = new graphics::UI::GuiSprite(m_enemiesLeftText, -60, 20, 60, 60);
	enemy_icon->LoadBitmapFromFile(L"icons/enemy_icon.png", false);
	m_sceneGuiElements.push_back(enemy_icon);
}

void LightVoxelMachinaApp::CheckForEnemyCollision()
{
	auto it = m_enemiesLeft.begin();

	while (it != m_enemiesLeft.end())
	{
		Enemy* enemy = *it;

		math::Vector3 distance = enemy->GetPosition() - m_player->GetPosition();

		if (math::Length(distance) <= 1.0f)
		{
			it = m_enemiesLeft.erase(it);
			enemiesLeft--;

			enemy->SetDetected();

			if (enemiesLeft == 0)
			{
				auto congratulationsSprite = new graphics::UI::GuiSprite(nullptr, 0, 0, graphics::g_windowHeight, graphics::g_windowHeight);
				congratulationsSprite->LoadBitmapFromFile(L"textures/congratulations.png", true);
				m_sceneGuiElements.push_back(congratulationsSprite);
			}
		}
		else
		{
			++it;
		}
	}
}

void LightVoxelMachinaApp::Cleanup(void)
{
}