#pragma once

#include "application.h"
#include "mathHelpers.h"
#include "camera.h"
#include "cameraController.h"
#include "graphics/lights.h"
#include "graphics/meshRenderer.h"
#include "graphics/renderPipeline.h"
#include "graphics/UI/guiText.h"
#include "graphics/UI/guiPanel.h"
#include "graphics/UI/guiSprite.h"

namespace graphics
{
	class RenderPipeline;
	struct MeshData;
	class MeshRenderer;
	class Material;
}

class LightVoxelMachinaApp : public IGameApp
{
public:
	virtual void Startup(void) override;

	virtual void Cleanup(void) override;

	virtual bool IsDone(void) override;

	virtual void Update(float deltaT) override;

	virtual void RenderScene(void) override;

	virtual void RenderUI(void) override;

	virtual void Resize(uint32_t width, uint32_t height) override;

private:

	Camera m_sceneCamera{};
	CameraController* m_cameraController{ nullptr };

	DirectionalLight m_sceneDirLight{};
	PointLight m_scenePointLight{};
	SpotLight m_sceneSpotLight{};

	std::unique_ptr<graphics::RenderPipeline> m_renderPipeline;

	std::vector<graphics::MeshData> m_sceneMeshes;
	std::vector<graphics::MeshRenderer> m_sceneMeshRenderer;

	graphics::UI::GuiText* m_guiText;
	graphics::UI::GuiPanel* m_guiPanel;
	graphics::UI::GuiSprite* m_sprite;

	graphics::Texture2D* m_skybox;

	math::Vector3 mouse_position;
	std::wstring m_typedStuff{ L"Commands: " };

	bool m_isDone = false;

private:
	void CreateCamera();
	void CreateLights();
	void CreateObjects();
	void CreateGUI();
};