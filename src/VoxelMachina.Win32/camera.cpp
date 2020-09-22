#include "pch.h"
#include "camera.h"
#include "graphics/coreGraphics.h"

using namespace DirectX;
using namespace math;

static Camera* g_mainCamera;

Camera* Camera::MainCamera()
{
	return g_mainCamera;
}

void Camera::SetAsMainCamera()
{
	g_mainCamera = this;
	this->SetRenderTarget(graphics::g_ScreenRenderTargetView);
	this->SetDepthStencil(graphics::g_ScreenDepthStencilView);
}

void Camera::SetLookDirection(math::Vector3 forward, math::Vector3 up)
{
	// Given, but ensure normalization
	Scalar forwardLenSq = LengthSquare(forward);
	forward = Select(forward * RecipSqrt(forwardLenSq), -Vector3(0, 0, 1), forwardLenSq < Scalar(0.000001f));

	// Deduce a valid, orthogonal right vector
	Vector3 right = Cross(forward, up);
	Scalar rightLenSq = LengthSquare(right);
	right = Select(right * RecipSqrt(rightLenSq), Quaternion(Vector3(0, 1, 0), -XM_PIDIV2) * forward, rightLenSq < Scalar(0.000001f));

	// Compute actual up vector
	up = Cross(right, forward);

	// Finish constructing basis
	m_Basis = Matrix3(right, up, -forward);
	m_CameraToWorld.SetRotation(Quaternion(m_Basis));
}

void Camera::UpdateProjectionMatrix()
{
	XMMATRIX V = XMMatrixPerspectiveFovLH(m_VerticalFOV, m_AspectRatio, m_NearClip, m_FarClip);
	m_projectionMatrix = math::Matrix4{ V };
	m_viewFrustum = math::Frustum(m_projectionMatrix);
	m_worldFrustum = math::Invert(m_viewMatrix) * m_viewFrustum;
}

void Camera::UpdateViewMatrix()
{
	m_viewMatrix = Matrix4(~m_CameraToWorld);
}

void Camera::UpdateViewProjectionMatrix()
{
	XMMATRIX viewProjection = DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix);
	m_viewProjectionMatrix = math::Matrix4{ viewProjection };
}