#include "pch.h"
#include "LightVoxelMachina.h"
#include "application.h"
#include "graphics/meshRenderer.h"
#include "graphics/renderPipeline.h"
#include "graphics/coreGraphics.h"
#include "input.h"
#include "colors.h"

#include "SimpleLightPS.h"
#include "SimpleLightVS.h"

void LightVoxelMachinaApp::Startup(void)
{
	graphics::MeshData sphere;
	graphics::MeshData::CreateSphere(1, 50, 20, sphere);

	graphics::MeshData quad;
	graphics::MeshData::CreateGrid(100, 100, 20, 20, quad);

	m_sceneMeshes.push_back(sphere);

	graphics::Material material1{};
	material1.Ambient = colors::cyan;
	material1.Diffuse = colors::cyan;
	material1.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 18.0f);

	auto meshRenderer1 = graphics::MeshRenderer(sphere, material1, math::Vector3(0, 2, 0), math::Quaternion());
	m_sceneMeshRenderer.push_back(meshRenderer1);

	graphics::Material material2{};
	material2.Ambient = colors::silver;
	material2.Diffuse = colors::silver;
	material2.Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 2);

	auto meshRenderer2 = graphics::MeshRenderer(quad, material2, math::Vector3(0, -3, 4), math::Quaternion());
	m_sceneMeshRenderer.push_back(meshRenderer2);

	m_renderPipeline = std::make_unique<graphics::RenderPipeline>();

	m_sceneCamera = Camera();
	math::Vector3 cameraPosition{ 0.0f, 0.0f, 6.0f };
	math::Vector3 target{ 0, 0, 0 };
	math::Vector3 up{ 0.0f, 1.0f, 0.0f };
	m_sceneCamera.SetEyeAtUp(cameraPosition, target, up);

	m_cameraController = new CameraController(m_sceneCamera, math::Vector3(0, 1, 0));

	auto aspectRation = static_cast<float>(graphics::g_windowWidth) / static_cast<float>(graphics::g_windowHeight);
	m_sceneCamera.SetPerspectiveMatrix(0.25f * math::Pi, aspectRation, 1.0f, 1000.0f);
	m_sceneCamera.Update();
	m_renderPipeline->LoadShader(g_pSimpleLightVS, sizeof(g_pSimpleLightVS), g_pSimpleLightPS, sizeof(g_pSimpleLightPS));

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
	m_scenePointLight.Intensity = 0.5;

	m_sceneSpotLight = SpotLight{};
	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	m_sceneSpotLight.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_sceneSpotLight.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_sceneSpotLight.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_sceneSpotLight.Att = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_sceneSpotLight.Spot = 96.0f;
	m_sceneSpotLight.Range = 10000.0f;
	m_sceneSpotLight.Position = DirectX::XMFLOAT3(2.0f, 3.0f, 0.0f);
	m_sceneSpotLight.Intensity = 0;

	m_guiPanel = new graphics::UI::GuiPanel(nullptr, 0, 0, 500, 500);
	m_guiPanel->SetColor(D2D1::ColorF{ D2D1::ColorF::White, 0.5f });

	//Create GUI Text
	m_guiText = new graphics::UI::GuiText(m_guiPanel, 0, 0, 300, 200, 50);
	m_guiText->SetColor(D2D1::ColorF::Black);
	m_guiText->SetText(L"Hello world");

	m_sprite = new graphics::UI::GuiSprite(nullptr, 0, -200, 200, 200);
	m_sprite->LoadBitmapFromFile(L"icon.png", false);
	m_sprite->SetAnchorType(graphics::UI::ParentAnchorType::BottomLeft);
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

void LightVoxelMachinaApp::Cleanup(void)
{
}