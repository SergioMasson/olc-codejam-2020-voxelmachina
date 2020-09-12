#pragma once

#include "application.h"
#include "mathHelpers.h"
#include "camera.h"
#include "cameraController.h"
#include "enemy.h"
#include "playerController.h"
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

	Light m_scenePointLight{};
	Light m_sceneSpotLight{};

	std::unique_ptr<graphics::RenderPipeline> m_renderPipeline;
	std::vector<graphics::MeshRenderer*> m_sceneMeshRenderer;
	std::vector<graphics::UI::DrawableElement*> m_sceneGuiElements;
	std::vector<Enemy*> m_enemiesLeft;

	graphics::UI::GuiText* m_informationText;
	graphics::UI::GuiText* m_counterText;
	graphics::UI::GuiText* m_enemiesLeftText;

	float m_time{ 0 };
	uint32_t enemiesLeft;

	graphics::MeshRenderer* m_player;
	PlayerController* m_playerController;
	CameraController* m_cameraController;

	graphics::Texture2D* m_skybox;
	math::Vector3 mouse_position;
	bool m_isDone = false;
	bool m_firstFrame = true;

private:
	void CreateCamera();
	void CreateLights();
	void CreateObjects();
	void CreateEnemy(graphics::MeshData* enemyData, graphics::Texture2D* enemyTexture, graphics::Texture2D* enemyNormal, graphics::Texture2D* detectedTexture, graphics::Texture2D* emissionMap);
	void CreatePilars(graphics::MeshData* pilarData, graphics::Texture2D* pilarTexture, graphics::Texture2D* pilarNormal, graphics::Texture2D* emissionMap);
	void CreateGUI();

	void CheckForEnemyCollision();
};