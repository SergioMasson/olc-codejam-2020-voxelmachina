#include "pch.h"
#include "camera.h"

using namespace DirectX;

void Camera::UpdateProjectionMatrix()
{
	XMMATRIX V = XMMatrixPerspectiveFovLH(m_VerticalFOV, m_AspectRatio, m_NearClip, m_FarClip);
	m_projectionMatrix = math::Matrix4{ V };
	UpdateViewProjectionMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMMATRIX V = DirectX::XMMatrixLookAtLH(m_position, m_lookDir, m_up);
	m_viewMatrix = math::Matrix4{ V };
	UpdateViewProjectionMatrix();
}

void Camera::UpdateViewProjectionMatrix()
{
	XMMATRIX viewProjection = DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix);
	m_viewProjectionMatrix = math::Matrix4{ viewProjection };
}