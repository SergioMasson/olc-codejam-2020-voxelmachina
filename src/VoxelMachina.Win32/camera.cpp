#include "pch.h"
#include "camera.h"

using namespace DirectX;
using namespace math;

void Camera::SetLookDirection(math::Vector3 forward, math::Vector3 up)
{
	forward = Normalize(forward);
	up = Normalize(up);

	// Deduce a valid, orthogonal right vector
	Vector3 right = Normalize(Cross(forward, up));

	// Compute actual up vector
	up = Normalize(Cross(forward, right));

	// Finish constructing basis
	m_Basis = Matrix3(right, up, -forward);
	m_CameraToWorld.SetRotation(Quaternion(m_Basis));
}

void Camera::UpdateProjectionMatrix()
{
	XMMATRIX V = XMMatrixPerspectiveFovLH(m_VerticalFOV, m_AspectRatio, m_NearClip, m_FarClip);
	m_projectionMatrix = math::Matrix4{ V };
}

void Camera::UpdateViewMatrix()
{
	//This works
	//m_viewMatrix = math::Matrix4{ V };

	m_viewMatrix = Matrix4(~m_CameraToWorld);
}

void Camera::UpdateViewProjectionMatrix()
{
	XMMATRIX viewProjection = DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix);
	m_viewProjectionMatrix = math::Matrix4{ viewProjection };
}