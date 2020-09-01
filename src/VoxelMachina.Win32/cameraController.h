#pragma once

#include "camera.h"

class CameraController
{
public:
	CameraController(Camera& camera, math::Vector3 worldUp);

	void Update(float dt);

	void SlowMovement(bool enable) { m_FineMovement = enable; }
	void SlowRotation(bool enable) { m_FineRotation = enable; }

	void EnableMomentum(bool enable) { m_Momentum = enable; }

	math::Vector3 GetWorldEast() { return m_WorldEast; }
	math::Vector3 GetWorldUp() { return m_WorldUp; }
	math::Vector3 GetWorldNorth() { return m_WorldNorth; }
	float GetCurrentHeading() { return m_CurrentHeading; }
	float GetCurrentPitch() { return m_CurrentPitch; }

	void SetCurrentHeading(float heading) { m_CurrentHeading = heading; }
	void SetCurrentPitch(float pitch) { m_CurrentPitch = pitch; }

private:
	CameraController& operator=(const CameraController&) { return *this; }

	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

	math::Vector3 m_WorldUp;
	math::Vector3 m_WorldNorth;
	math::Vector3 m_WorldEast;
	Camera& m_TargetCamera;
	float m_HorizontalLookSensitivity;
	float m_VerticalLookSensitivity;
	float m_MoveSpeed;
	float m_StrafeSpeed;
	float m_MouseSensitivityX;
	float m_MouseSensitivityY;
	float m_CurrentHeading;
	float m_CurrentPitch;

	bool m_FineMovement;
	bool m_FineRotation;
	bool m_Momentum;

	float m_LastYaw;
	float m_LastPitch;
	float m_LastForward;
	float m_LastStrafe;
	float m_LastAscent;
};