#include "pch.h"
#include "LightVoxelMachina.h"
#include "application.h"
#include "graphics/meshRenderer.h"
#include "graphics/renderPipeline.h"
#include "graphics/coreGraphics.h"
#include "graphics/texture2D.h"
#include "audio/audioCore.h"
#include "math/random.h"
#include "input.h"
#include "colors.h"

#include "pbrPS.h"
#include "pbrVS.h"
#include <string>

#define ENEMY_COUNT 10
#define PILAR_COUNT 100
#define WORLD_X  100.0f
#define WORLD_Y  100.0f

graphics::MeshData quad;
graphics::MeshData playerCharacter;
graphics::MeshData enemyMesh;
graphics::MeshData pilarMesh;
graphics::MeshData trophyMesh;
graphics::MeshData worldMesh;

LightData dirLight;

float currentTime;

void LightVoxelMachinaApp::Startup(void)
{
	enemiesLeft = ENEMY_COUNT;

	//Create the render pipeline
	m_renderPipeline = std::make_unique<graphics::RenderPipeline>();
	m_renderPipeline->LoadShader(g_ppbrVS, sizeof(g_ppbrVS), g_ppbrPS, sizeof(g_ppbrPS));

	CreateLights();
	CreateObjects();
	CreateGUI();
	CreateCamera();

	m_playerController = new PlayerController(math::Vector3(0, 1, 0), m_player, &m_sceneCamera);
	m_cameraController = new CameraController(m_sceneCamera, math::Vector3(0, 1, 0));

	audio::PlayAudioFile(L"audioFiles/test.wav", true);
}

bool LightVoxelMachinaApp::IsDone()
{
	return m_isDone;
}

void LightVoxelMachinaApp::Update(float deltaT, float totalTime)
{
	currentTime = totalTime;

	if (m_firstFrame)
	{
		m_firstFrame = false;
		return;
	}

	m_isDone = Input::IsPressed(KeyCode::Key_escape) || (Input::IsPressed(KeyCode::Key_return) && enemiesLeft == 0);

	auto oldPlayerPosition = m_player->GetPosition();

	m_playerController->Update(deltaT);

	if (CheckPillarCollision() || !CheckIfInsideScene())
		m_player->SetPosition(oldPlayerPosition);

	//m_cameraController->Update(deltaT);

	CheckForEnemyCollision();

	math::Vector3 lightPosition1 = m_player->GetPosition() + (m_player->GetRotation() * math::Vector3{ 0, 1.5f, 1.5f });
	math::Vector3 lightPosition2 = m_player->GetPosition() + (m_player->GetRotation() * math::Vector3{ 0, 5.5f, -1.0f });
	math::Vector3 playerFoward = math::Matrix3{ m_player->GetRotation() }.GetZ();

	DirectX::XMStoreFloat3(&m_scenePointLight.Position, lightPosition1);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Position, lightPosition2);
	DirectX::XMStoreFloat3(&m_sceneSpotLight.Direction, playerFoward);

	if (m_enemiesLeft.size() != 0)
		m_time += deltaT;
	else
	{
		m_trophy->SetRotation(m_trophy->GetRotation() * math::Quaternion(math::Vector3(0, 1, 0), deltaT * 2));
		m_trophy->SetPosition(m_trophy->GetPosition() + math::Vector3(0.0f, 0.8f * sin(1.5f * totalTime) * 0.005f, 0));
	}

	m_counterText->SetText(L"TOTAL TIME: " + std::to_wstring(m_time));
	m_enemiesLeftText->SetText(L" X " + std::to_wstring(enemiesLeft));
}

void LightVoxelMachinaApp::RenderScene(void)
{
	m_renderPipeline->StartRender(&m_sceneCamera);

	for (auto renderer : m_sceneMeshRenderer)
		m_renderPipeline->RenderMesh(*renderer);
}

//TODO(Sergio): Implement those things later.
void LightVoxelMachinaApp::RenderUI(void)
{
	Microsoft::WRL::ComPtr<ID2D1Image> renderTarget;

	graphics::g_d2dDeviceContext->GetTarget(renderTarget.GetAddressOf());

	if (renderTarget != nullptr)
	{
		graphics::g_d2dDeviceContext->BeginDraw();

		//Draw all GUI elements.
		for (auto guiElement : m_sceneGuiElements)
			guiElement->Draw();

		ASSERT_SUCCEEDED(graphics::g_d2dDeviceContext->EndDraw());
	}
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
	math::Vector3 cameraPosition{ 0.0f, 2.0f, 10.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };
	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 30.0f);
	m_sceneCamera.Update();
}

void LightVoxelMachinaApp::CreateLights()
{
	m_scenePointLight = LightData{};
	// Point light--position is changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = 0.01f;
	m_scenePointLight.Color = Color::Aqua;
	m_scenePointLight.Range = 7;
	m_scenePointLight.Position = DirectX::XMFLOAT3(0.0f, 3.0f, 5.0f);
	m_scenePointLight.Intensity = 2.0f;
	m_scenePointLight.LightType = POINT_LIGHT;

	m_renderPipeline->AddLight(&m_scenePointLight);

	m_sceneSpotLight = LightData{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_sceneSpotLight.Color = Color::White;
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 15.0f;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_sceneSpotLight.Intensity = 0.15f;
	m_sceneSpotLight.LightType = POINT_LIGHT;
	//m_renderPipeline->AddLight(&m_sceneSpotLight);
}

void LightVoxelMachinaApp::CreateObjects()
{
	graphics::MeshData::CreateGrid(100, 100, 20, 20, quad);

	graphics::MeshData::LoadFromOBJFile(L"models/littleRobot.model", playerCharacter);

	graphics::MeshData::LoadFromOBJFile(L"models/enemy.model", enemyMesh);

	graphics::MeshData::LoadFromOBJFile(L"models/trophy.model", trophyMesh);

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

	auto defaultEmissionMap = new graphics::Texture2D(L"textures/defaultEmissionMap.dds");
	auto playerEmissionMap = new graphics::Texture2D(L"textures/newEmissionMap.dds");

	graphics::Material playerMaterial{};
	playerMaterial.Metalness = 0.2f;
	playerMaterial.Roughness = 0.2f;
	playerMaterial.Color = Color::White;

	m_player = new GameObject();
	auto playerRenderer = m_player->AddMeshRenderer(&playerCharacter, playerMaterial);
	playerRenderer->SetAlbedoTexture(playerTexture);
	playerRenderer->SetNormalMap(normalMap);
	playerRenderer->SetEmissionMap(playerEmissionMap);
	m_sceneMeshRenderer.push_back(playerRenderer);

	m_activeGameObjects.push_back(m_player);

	graphics::Material trophyMaterial{};
	trophyMaterial.Metalness = 0.9f;
	trophyMaterial.Roughness = 0.7f;
	trophyMaterial.Color = Color::White;

	m_trophy = new GameObject(math::Vector3(0, 1.2f, 0));
	auto trophyRenderer = m_trophy->AddMeshRenderer(&trophyMesh, trophyMaterial);
	trophyRenderer->SetAlbedoTexture(playerTexture);
	trophyRenderer->SetNormalMap(normalMap);
	trophyRenderer->SetNormalMap(normalMap);
	trophyRenderer->SetEmissionMap(defaultEmissionMap);
	m_trophy->SetParent(m_player);

	m_activeGameObjects.push_back(m_trophy);

	CreateEnemy(&enemyMesh, enemyTexture, normalMap, enemyDetectedTexture, defaultEmissionMap);
	CreatePilars(&pilarMesh, pilarTexture, pilarNormal, defaultEmissionMap);

	graphics::Material floorMaterial{};
	floorMaterial.Color = Color::White;
	floorMaterial.Metalness = 0.3f;
	floorMaterial.Roughness = 0.1f;

	m_floor = new GameObject();
	auto floorRenderer = m_floor->AddMeshRenderer(&quad, floorMaterial);
	floorRenderer->SetAlbedoTexture(floorTexture);
	floorRenderer->SetNormalMap(floorNormalMap);
	floorRenderer->SetEmissionMap(defaultEmissionMap);
	floorRenderer->SetTextureScale(20, 20);

	m_sceneMeshRenderer.push_back(floorRenderer);
	m_activeGameObjects.push_back(m_floor);
}

void LightVoxelMachinaApp::CreateEnemy(graphics::MeshData* enemyData, graphics::Texture2D* enemyTexture, graphics::Texture2D* enemyNormal, graphics::Texture2D* detectedTexture, graphics::Texture2D* emissionMap)
{
	graphics::Material material1{};
	material1.Color = Color::White;
	material1.Metalness = 0.2f;
	material1.Roughness = 0.1f;

	math::RandomNumberGenerator random{};

	for (size_t i = 0; i < ENEMY_COUNT; i++)
	{
		float randomX = random.NextFloat();
		float randomY = random.NextFloat();

		float enemyX = (randomX * WORLD_X) - ((WORLD_X) / 2.0f);
		float enemyY = (randomY * WORLD_Y) - ((WORLD_Y) / 2.0f);
		auto enemyPosition = math::Vector3(enemyX, 0, enemyY);

		auto enemy = new GameObject(enemyPosition, math::Quaternion());

		auto enemyRenderer = enemy->AddMeshRenderer(enemyData, material1);
		enemyRenderer->SetAlbedoTexture(enemyTexture);
		enemyRenderer->SetNormalMap(enemyNormal);
		enemyRenderer->SetEmissionMap(emissionMap);
		m_sceneMeshRenderer.push_back(enemyRenderer);
		m_enemiesLeft.push_back(new Enemy(enemy, detectedTexture));
	}
}

void LightVoxelMachinaApp::CreatePilars(graphics::MeshData* pilarData, graphics::Texture2D* pilarTexture, graphics::Texture2D* pilarNormal, graphics::Texture2D* emissionMap)
{
	graphics::Material material1{};
	material1.Color = Color::White;
	material1.Metalness = 0.3f;
	material1.Roughness = 0.1f;

	math::RandomNumberGenerator random{};

	for (size_t i = 0; i < PILAR_COUNT; i++)
	{
		float pilarX = 0.0f;
		float pilarY = 0.0f;

		math::Vector3 distance;

		auto pilar = new GameObject(math::Vector3(0, 5, 0), math::Quaternion());
		auto pilarRenderer = pilar->AddMeshRenderer(pilarData, material1);
		pilarRenderer->SetAlbedoTexture(pilarTexture);
		pilarRenderer->SetNormalMap(pilarNormal);
		pilarRenderer->SetTextureScale(3, 5);
		pilarRenderer->SetEmissionMap(emissionMap);

		bool overlapingEnemy = false;

		do
		{
			overlapingEnemy = false;

			pilarX = (random.NextFloat() * (WORLD_X - 3.0f)) - ((WORLD_X - 3.0f) / 2.0f);
			pilarY = (random.NextFloat() * (WORLD_Y - 3.0f)) - ((WORLD_Y - 3.0f) / 2.0f);

			pilar->SetPosition(math::Vector3(pilarX, 5, pilarY));
			auto pilarBound = pilarRenderer->WBoudingBox();

			overlapingEnemy = pilarBound.IsOverlaping(m_player->GetMeshRenderer()->WBoudingBox());

			for (auto enemy : m_enemiesLeft)
				overlapingEnemy = overlapingEnemy || enemy->WBoudingBox().IsOverlaping(pilarBound);

			for (auto otherPilar : m_pilars)
				overlapingEnemy = overlapingEnemy || otherPilar->WBoudingBox().IsOverlaping(pilarBound);
		} while (math::Length(distance) <= 10.0f && overlapingEnemy);

		m_sceneMeshRenderer.push_back(pilarRenderer);
		m_activeGameObjects.push_back(pilar);
		m_pilars.push_back(pilarRenderer);
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

		if (enemy->WBoudingBox().IsOverlaping(m_player->GetMeshRenderer()->WBoudingBox()))
		{
			it = m_enemiesLeft.erase(it);
			enemiesLeft--;

			enemy->SetDetected();

			LightData* light = new LightData();
			*light = CreatePointLight(Color::MediumVioletRed, enemy->GetPosition() + math::Vector3(0, 0.5f, 0), 7.0f, 0.0f, 10.0f);

			m_renderPipeline->AddLight(light);
			audio::PlayAudioFile(L"audioFiles/enemy.wav");

			if (enemiesLeft == 0)
			{
				m_sceneGuiElements.clear();

				auto congratulationsSprite = new graphics::UI::GuiSprite(nullptr, 0, 0, 0, 0);
				congratulationsSprite->LoadBitmapFromFile(L"icons/trophy.png", true);

				float imageYPosition = graphics::g_windowHeight - congratulationsSprite->GetHeight();

				m_counterText->SetFontSize(100);

				m_sceneGuiElements.push_back(m_counterText);

				congratulationsSprite->SetLocalPosition(0, imageYPosition / 2.0f);

				m_sceneGuiElements.push_back(congratulationsSprite);
				m_sceneMeshRenderer.push_back(m_trophy->GetMeshRenderer());

				auto pressEnterText = new graphics::UI::GuiText(nullptr, 0.0f, 0.0f, static_cast<float>(graphics::g_windowWidth), -200.0f, 40.0f);
				pressEnterText->SetColor(Color::White);
				pressEnterText->SetAnchorType(graphics::UI::ParentAnchorType::BottomLeft);
				pressEnterText->SetText(L"Press enter to EXIT");
				pressEnterText->SetTextAlignment(TextAlignment::Center);
				m_sceneGuiElements.push_back(pressEnterText);
			}
		}
		else
		{
			++it;
		}
	}
}

bool LightVoxelMachinaApp::CheckPillarCollision()
{
	for (auto pilar : m_pilars)
	{
		if (pilar->WBoudingBox().IsOverlaping(m_player->GetMeshRenderer()->WBoudingBox()))
			return true;
	}
	return false;
}

bool LightVoxelMachinaApp::CheckIfInsideScene()
{
	return m_floor->GetMeshRenderer()->WBoudingBox().IsPointInside(m_player->GetPosition());
}

void LightVoxelMachinaApp::Cleanup(void)
{
}