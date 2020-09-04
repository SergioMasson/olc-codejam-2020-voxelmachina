#include "pch.h"
#include "input.h"
#include "cameraController.h"

CameraController::CameraController(Camera& camera, math::Vector3 worldUp) : m_TargetCamera{ camera }
{
	m_WorldUp = Normalize(worldUp);
	m_WorldNorth = Normalize(Cross(m_WorldUp, math::Vector3(1, 0, 0)));
	m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

	m_HorizontalLookSensitivity = 2.0f;
	m_VerticalLookSensitivity = 2.0f;
	m_MoveSpeed = 10.0f;
	m_StrafeSpeed = 10.0f;
	m_MouseSensitivityX = 0.05f;
	m_MouseSensitivityY = 0.05f;

	m_CurrentPitch = Sin(Dot(camera.GetForwardVec(), m_WorldUp));

	math::Vector3 forward = Normalize(Cross(m_WorldUp, camera.GetRightVec()));
	m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

	m_FineMovement = false;
	m_FineRotation = false;
	m_Momentum = true;

	m_LastYaw = 0.0f;
	m_LastPitch = 0.0f;
	m_LastForward = 0.0f;
	m_LastStrafe = 0.0f;
	m_LastAscent = 0.0f;
}

void CameraController::Update(float deltaTime)
{
	(deltaTime);

	float timeScale = 1.0f;

	if (Input::IsFirstPressed(Input::KeyCode::LThumbClick) || Input::IsFirstPressed(Input::KeyCode::Key_lshift))
		m_FineMovement = !m_FineMovement;

	if (Input::IsFirstPressed(Input::KeyCode::RThumbClick))
		m_FineRotation = !m_FineRotation;

	float speedScale = (m_FineMovement ? 0.1f : 1.0f) * timeScale;
	float panScale = (m_FineRotation ? 0.5f : 1.0f) * timeScale;

	float yaw = Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogRightStickX) * m_HorizontalLookSensitivity * panScale;
	float pitch = Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogRightStickY) * m_VerticalLookSensitivity * panScale;

	float forward = m_MoveSpeed * speedScale * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(Input::KeyCode::Key_s) ? deltaTime : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_w) ? -deltaTime : 0.0f)
		);

	float strafe = m_StrafeSpeed * speedScale * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(Input::KeyCode::Key_d) ? deltaTime : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_a) ? -deltaTime : 0.0f)
		);

	float ascent = m_StrafeSpeed * speedScale * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogRightTrigger) -
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftTrigger) +
		(Input::IsPressed(Input::KeyCode::Key_e) ? deltaTime : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_q) ? -deltaTime : 0.0f)
		);

	if (m_Momentum)
	{
		ApplyMomentum(m_LastYaw, yaw, deltaTime);
		ApplyMomentum(m_LastPitch, pitch, deltaTime);
		ApplyMomentum(m_LastForward, forward, deltaTime);
		ApplyMomentum(m_LastStrafe, strafe, deltaTime);
		ApplyMomentum(m_LastAscent, ascent, deltaTime);
	}

	float mouseX = 0;
	float mouseY = 0;

	if (Input::IsPressed(Input::KeyCode::MouseRigth))
	{
		mouseX = Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseX);
		mouseY = Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseY);
	}

	//// don't apply momentum to mouse inputs
	yaw += mouseX * m_MouseSensitivityX;
	pitch += mouseY * m_MouseSensitivityY;

	m_CurrentPitch += pitch;
	m_CurrentPitch = DirectX::XMMin(DirectX::XM_PIDIV2, m_CurrentPitch);
	m_CurrentPitch = DirectX::XMMax(-DirectX::XM_PIDIV2, m_CurrentPitch);

	m_CurrentHeading -= yaw;
	if (m_CurrentHeading > DirectX::XM_PI)
		m_CurrentHeading -= DirectX::XM_2PI;
	else if (m_CurrentHeading <= -DirectX::XM_PI)
		m_CurrentHeading += DirectX::XM_2PI;

	math::Matrix3 orientation = math::Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * math::Matrix3::MakeYRotation(m_CurrentHeading) * math::Matrix3::MakeXRotation(m_CurrentPitch);
	math::Vector3 position = orientation * math::Vector3(strafe, ascent, -forward) + m_TargetCamera.GetPosition();

	m_TargetCamera.SetTransform(math::Transform(orientation, position));
	m_TargetCamera.Update();
}

void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	float blendedValue;
	if (math::Abs(newValue) > math::Abs(oldValue))
		blendedValue = math::Lerp(newValue, oldValue, math::Pow(0.6f, deltaTime * 60.0f));
	else
		blendedValue = math::Lerp(newValue, oldValue, math::Pow(0.8f, deltaTime * 60.0f));
	oldValue = blendedValue;
	newValue = blendedValue;
}