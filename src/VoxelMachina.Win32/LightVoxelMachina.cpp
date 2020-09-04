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

	char key = 0;

	if (Input::IsPressed(Input::KeyCode::Key_return))
		m_typedStuff = L"Command: ";
	else if (Input::IsPressed(Input::KeyCode::Key_back) && m_typedStuff.size() > 1)
		m_typedStuff.erase(m_typedStuff.size() - 1, m_typedStuff.size());
	else if (Input::TryGetLastKeyboardType(key))
		m_typedStuff += key;

	m_guiText->SetText(m_typedStuff);
}

void LightVoxelMachinaApp::RenderScene(void)
{
	m_renderPipeline->StartRender(&m_sceneCamera, m_sceneDirLight, m_sceneSpotLight, m_scenePointLight);

	for (auto& renderer : m_sceneMeshRenderer)
		m_renderPipeline->RenderMesh(renderer);
}

//TODO(Sergio): Implement those things later.
void LightVoxelMachinaApp::RenderUI(void)
{
	graphics::g_d2dDeviceContext->BeginDraw();

	m_guiPanel->Draw();
	m_guiText->Draw();
	m_sprite->Draw();

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
	math::Vector3 cameraPosition{ 0.0f, 0.0f, 6.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };
	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);

	m_cameraController = new CameraController(m_sceneCamera, math::Vector3(0, 1, 0));

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	m_sceneCamera.Update();
}

void LightVoxelMachinaApp::CreateLights()
{
	m_sceneDirLight = DirectionalLight{};
	//Directional light.
	m_sceneDirLight.Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_sceneDirLight.Diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_sceneDirLight.Specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 3.0f);
	m_sceneDirLight.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	m_sceneDirLight.Intensity = 1;

	m_scenePointLight = PointLight{};
	//// Point light--position is changed every frame to animate in UpdateScene function.
	m_scenePointLight.Ambient = DirectX::XMFLOAT4(0, 0, 0, 1.0f);
	m_scenePointLight.Diffuse = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_scenePointLight.Specular = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_scenePointLight.Att = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_scenePointLight.Range = 25.0f;
	m_scenePointLight.Position = DirectX::XMFLOAT3(6.0f, 3.0f, 0.0f);
	m_scenePointLight.Intensity = 1;

	m_sceneSpotLight = SpotLight{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_sceneSpotLight.Ambient = DirectX::XMFLOAT4(1, 1, 1, 1);
	m_sceneSpotLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_sceneSpotLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Att = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 10000.0f;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Intensity = 0;
}

void LightVoxelMachinaApp::CreateObjects()
{
	graphics::MeshData sphere;
	graphics::MeshData::CreateSphere(1, 50, 20, sphere);

	graphics::MeshData quad;
	graphics::MeshData::CreateGrid(30, 30, 20, 20, quad);

	graphics::MeshData cylinder;
	graphics::MeshData::CreateCylinder(1, 1, 5, 50, 20, cylinder);

	m_sceneMeshes.push_back(sphere);
	auto fire = new graphics::Texture2D(L"textures/fire.dds");
	auto water = new graphics::Texture2D(L"textures/water1.dds");
	auto grass = new graphics::Texture2D(L"textures/grass.dds");
	auto bricks = new graphics::Texture2D(L"textures/bricks.dds");

	graphics::Material material1{};
	material1.Ambient = colors::white;
	material1.Diffuse = colors::white;
	material1.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 18.0f);

	auto sphereFire = graphics::MeshRenderer(sphere, material1, math::Vector3(0, 2, 0), math::Quaternion());
	sphereFire.SetAlbedoTexture(fire);
	m_sceneMeshRenderer.push_back(sphereFire);

	auto sphereWater = graphics::MeshRenderer(sphere, material1, math::Vector3(4, 2, 0), math::Quaternion());
	sphereWater.SetAlbedoTexture(water);
	m_sceneMeshRenderer.push_back(sphereWater);

	auto sphereGrass = graphics::MeshRenderer(sphere, material1, math::Vector3(8, 2, 0), math::Quaternion());
	sphereGrass.SetAlbedoTexture(grass);
	m_sceneMeshRenderer.push_back(sphereGrass);

	auto brick = graphics::MeshRenderer(cylinder, material1, math::Vector3(5, 0, 5), math::Quaternion());
	brick.SetAlbedoTexture(bricks);
	m_sceneMeshRenderer.push_back(brick);

	auto texture2 = new graphics::Texture2D(L"textures/checkboard.dds");

	graphics::Material material2{};
	material2.Ambient = colors::silver;
	material2.Diffuse = colors::silver;
	material2.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2);

	auto meshRenderer2 = graphics::MeshRenderer(quad, material2, math::Vector3(0, -3, 4), math::Quaternion());
	meshRenderer2.SetAlbedoTexture(texture2);
	meshRenderer2.SetTextureScale(10, 10);

	m_sceneMeshRenderer.push_back(meshRenderer2);
}

void LightVoxelMachinaApp::CreateGUI()
{
	m_guiPanel = new graphics::UI::GuiPanel(nullptr, 0, 0, 500, 500);
	m_guiPanel->SetColor(D2D1::ColorF{ D2D1::ColorF::White, 0.5f });

	//Create GUI Text
	m_guiText = new graphics::UI::GuiText(m_guiPanel, 0, 0, 300, 200, 50);
	m_guiText->SetColor(D2D1::ColorF::Black);
	m_guiText->SetText(L"Hello world");

	m_sprite = new graphics::UI::GuiSprite(nullptr, -200, -200, 200, 200);
	m_sprite->LoadBitmapFromFile(L"icons/icon.png", false);
	m_sprite->SetAnchorType(graphics::UI::ParentAnchorType::BottomRight);
}

void LightVoxelMachinaApp::Cleanup(void)
{
}