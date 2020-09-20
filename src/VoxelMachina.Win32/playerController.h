#pragma once

#include "graphics/meshRenderer.h"
#include "camera.h"
#include "gameObject.h"

class PlayerController : public BehaviourComponent
{
public:
	PlayerController(GameObject* gameObject, math::Vector3 worldUp);
	void Update(float deltaT) override;

private:

	math::Vector3 m_WorldUp;
	math::Vector3 m_WorldNorth;
	math::Vector3 m_WorldEast;
	math::Vector3 m_cameraOffset;

	float m_MoveSpeed;
	float m_RotationSpeed;
	float m_mouseCameraRotationSpeed;
	float m_xboxCamRotationSpeed;
	float m_cameraZoomSpeed;

	float m_CurrentHeading;

	float m_MouseSensitivityX;
	float m_MouseSensitivityY;

	float m_lastCameraRotationX;
	float m_lastCameraRotationY;

	float m_lastPlayerFoward;
	float m_lastPlayerFowardX;

	float m_lastCameraDelta;

private:
	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);
};