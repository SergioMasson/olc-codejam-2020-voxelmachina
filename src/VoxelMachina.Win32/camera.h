#pragma once

#include "mathHelpers.h"

class Camera
{
public:
	Camera() : m_viewMatrix{}, m_projectionMatrix{}, m_viewProjectionMatrix{}{}

	// Public functions for controlling where the camera is and its orientation
	void SetEyeAtUp(math::Vector3 eye, math::Vector3 at, math::Vector3 up)
	{
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(eye, at, up);
		auto rotation = DirectX::XMQuaternionRotationMatrix(V);
		m_CameraToWorld.SetRotation(math::Quaternion(rotation));
		m_CameraToWorld.SetTranslation(eye);
		UpdateViewMatrix();
	}

	void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
	{
		m_VerticalFOV = verticalFovRadians;
		m_AspectRatio = aspectHeightOverWidth;
		m_NearClip = nearZClip;
		m_FarClip = farZClip;
	}

	void Update()
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
		UpdateViewProjectionMatrix();
	}

	void StoreViewProjectionMatrix(DirectX::XMFLOAT4X4* target)
	{
		XMStoreFloat4x4(target, m_viewProjectionMatrix);
	}

	float GetFOV() const { return m_VerticalFOV; }
	float GetAspectRatio() const { return m_AspectRatio; }
	float GetNearPlane() const { return m_NearClip; }
	float GetFarPlane() const { return m_FarClip; }

	void SetRotation(math::Quaternion basisRotation)
	{
		m_CameraToWorld.SetRotation(math::Quaternion(DirectX::XMQuaternionNormalize(basisRotation)));
	}

	const math::Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }

	void SetPosition(math::Vector3 worldPos)
	{
		m_CameraToWorld.SetTranslation(worldPos);
	}

	const math::Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

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

	math::Transform m_CameraToWorld;

	float m_VerticalFOV = 0;            // Field of view angle in radians
	float m_AspectRatio = 0;
	float m_NearClip = 0;
	float m_FarClip = 0;
};