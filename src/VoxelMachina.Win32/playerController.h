#pragma once

#include "graphics/meshRenderer.h"
#include "camera.h"

class PlayerController
{
public:
	PlayerController(math::Vector3 worldUp, graphics::MeshRenderer* playerMesh, Camera* sceneCamera, bool lockCamera = false);
	void Update(float deltaT);

private:

	math::Vector3 m_WorldUp;
	math::Vector3 m_WorldNorth;
	math::Vector3 m_WorldEast;

	math::Vector3 m_cameraOffset;

	graphics::MeshRenderer* m_playerMeshRenderer;
	Camera* m_sceneCamera;

	float m_MoveSpeed;
	float m_RotationSpeed;
	float m_mouseCameraRotationSpeed;
	float m_xboxCamRotationSpeed;

	float m_CurrentHeading;

	float m_MouseSensitivityX;
	float m_MouseSensitivityY;

	float m_lastCameraRotationX;
	float m_lastCameraRotationY;

	float m_lastPlayerFoward;
	float m_lastPlayerFowardX;

	bool m_lockCamera;

private:
	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);
};