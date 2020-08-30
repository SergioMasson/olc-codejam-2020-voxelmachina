#include "pch.h"
#include "camera.h"

using namespace DirectX;
using namespace math;

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