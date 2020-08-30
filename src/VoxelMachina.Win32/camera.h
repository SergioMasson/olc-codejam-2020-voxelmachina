#pragma once

#include "mathHelpers.h"

class Camera
{
public:
	Camera() : m_viewMatrix{}, m_projectionMatrix{}, m_viewProjectionMatrix{}{}

	// Public functions for controlling where the camera is and its orientation
	void SetEyeAtUp(math::Vector3 eye, math::Vector3 at, math::Vector3 up)
	{
		m_position = eye;
		m_up = up;
		m_lookDir = at;
		UpdateViewMatrix();
	}

	void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
	{
		m_VerticalFOV = verticalFovRadians;
		m_AspectRatio = aspectHeightOverWidth;
		m_NearClip = nearZClip;
		m_FarClip = farZClip;
		UpdateProjectionMatrix();
	}

	void StoreViewProjectionMatrix(DirectX::XMFLOAT4X4* target)
	{
		XMStoreFloat4x4(target, m_viewProjectionMatrix);
	}

	float GetFOV() const { return m_VerticalFOV; }
	float GetAspectRatio() const { return m_AspectRatio; }
	float GetNearPlane() const { return m_NearClip; }
	float GetFarPlane() const { return m_FarClip; }

	math::Vector3 GetPosition() const { return m_position; }
	math::Vector3 GetUp() const { return m_up; }
	math::Vector3 GetLookDirection() const { return m_lookDir; }

	void SetPosition(math::Vector3 value)
	{
		m_position = value;
		UpdateViewMatrix();
	}

	void SetLookDirection(math::Vector3 value)
	{
		DirectX::XMVECTOR temporaryValue = value;

		DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(temporaryValue);
		DirectX::XMVECTOR oldUP = m_up;

		DirectX::XMVECTOR znor = DirectX::XMVector3Cross(normalized, oldUP);
		znor = DirectX::XMVector3Normalize(znor);

		m_up = math::Vector3{ DirectX::XMVector3Cross(normalized, znor) };

		UpdateViewMatrix();
	}

	void SetFOV(float value)
	{
		m_VerticalFOV = value;
		UpdateProjectionMatrix();
	}

	void SetAspectRatio(float value)
	{
		m_AspectRatio = value;
		UpdateProjectionMatrix();
	}

	void SetNearPlane(float value)
	{
		m_NearClip = value;
		UpdateProjectionMatrix();
	}

	void SetFarPlane(float value)
	{
		m_FarClip = value;
		UpdateProjectionMatrix();
	}

private:
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	void UpdateViewProjectionMatrix();

private:
	math::Matrix4 m_viewMatrix;
	math::Matrix4 m_projectionMatrix;
	math::Matrix4 m_viewProjectionMatrix;

	float m_VerticalFOV = 0;            // Field of view angle in radians
	float m_AspectRatio = 0;
	float m_NearClip = 0;
	float m_FarClip = 0;

	math::Vector3 m_position{ 0, 0, 0 };
	math::Vector3 m_up{ 0, 0, 0 };
	math::Vector3 m_lookDir{ 0, 0, 0 };
};