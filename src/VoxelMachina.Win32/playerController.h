#pragma once

#include "graphics/meshRenderer.h"
#include "camera.h"

class PlayerController
{
public:
	PlayerController(math::Vector3 worldUp, graphics::MeshRenderer* playerMesh, Camera* sceneCamera);
	void Update(float deltaT);

private:

	math::Vector3 m_WorldUp;
	math::Vector3 m_WorldNorth;
	math::Vector3 m_WorldEast;

	graphics::MeshRenderer* m_playerMeshRenderer;
	Camera* m_sceneCamera;

	float m_MoveSpeed;
	float m_RotationSpeed;

	float m_CurrentHeading;

	float m_MouseSensitivityX;
	float m_MouseSensitivityY;

	float m_LastYaw = 0;
	float m_LastPitch = 0;
	float m_LastForward = 0;
	float m_LastStrafe = 0;
	float m_LastAscent = 0;

private:
	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);
};