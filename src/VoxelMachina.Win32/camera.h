#pragma once

#include "mathHelpers.h"
#include "math/frustum.h"

class Camera
{
public:
	Camera() :
		m_viewMatrix{},
		m_projectionMatrix{},
		m_viewProjectionMatrix{}, m_Basis{}
	{
		if (MainCamera() == nullptr)
			this->SetAsMainCamera();
	}

	static Camera* MainCamera();

	void SetAsMainCamera();

	// Public functions for controlling where the camera is and its orientation
	void SetEyeAtUp(math::Vector3 eye, math::Vector3 at, math::Vector3 up)
	{
		SetLookDirection(eye - at, up);
		SetPosition(eye);
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
		m_Basis = math::Matrix3(m_CameraToWorld.GetRotation());
	}

	const math::Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }

	void SetPosition(math::Vector3 worldPos)
	{
		m_CameraToWorld.SetTranslation(worldPos);
	}

	void SetTransform(const math::Transform& xform)
	{
		m_CameraToWorld = xform;
	}

	const math::Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

	const math::Vector3 GetRightVec() const { return m_Basis.GetX(); }
	const math::Vector3 GetUpVec() const { return m_Basis.GetY(); }
	const math::Vector3 GetForwardVec() const { return -m_Basis.GetZ(); }

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

	math::Matrix4 GetViewMatrix() const { return m_viewMatrix.Transpose(); }
	math::Matrix4 GetProjectionMatrix() const { return m_projectionMatrix.Transpose(); }
	math::Matrix4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix.Transpose(); }
	math::Frustum GetViewFrustum() const { return m_viewFrustum; };
	inline math::Frustum GetWorldFrustum() const { return m_worldFrustum; };

private:
	void SetLookDirection(math::Vector3 forward, math::Vector3 up);
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	void UpdateViewProjectionMatrix();

private:
	math::Matrix4 m_viewMatrix;
	math::Matrix4 m_projectionMatrix;
	math::Matrix4 m_viewProjectionMatrix;
	math::Transform m_CameraToWorld;

	// Redundant data cached for faster lookups.
	math::Matrix3 m_Basis;

	math::Frustum m_viewFrustum;
	math::Frustum m_worldFrustum;

	float m_VerticalFOV = 0;            // Field of view angle in radians
	float m_AspectRatio = 0;
	float m_NearClip = 0;
	float m_FarClip = 0;

public:
};