#include "pch.h"
#include "playerController.h"
#include "input.h"

PlayerController::PlayerController(math::Vector3 worldUp,
	graphics::MeshRenderer* meshRenderer,
	Camera* sceneCamera) : m_playerMeshRenderer{ meshRenderer }, m_sceneCamera{ sceneCamera }
{
	m_WorldUp = Normalize(worldUp);
	m_WorldNorth = Normalize(Cross(m_WorldUp, math::Vector3(1, 0, 0)));
	m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

	m_MoveSpeed = 10.0f;
	m_RotationSpeed = 1.0f;

	m_MouseSensitivityX = 0.1f;
	m_MouseSensitivityY = 0.05f;

	auto foward = -math::Matrix3{ m_sceneCamera->GetRotation() }.GetX();

	math::Vector3 forward = Normalize(Cross(m_WorldUp, foward));
	m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

	m_LastYaw = 0.0f;
	m_LastPitch = 0.0f;
	m_LastForward = 0.0f;
	m_LastStrafe = 0.0f;
	m_LastAscent = 0.0f;
}

void PlayerController::Update(float deltaT)
{
	float forward = m_MoveSpeed * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(Input::KeyCode::Key_s) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_w) ? -deltaT : 0.0f)
		);

	math::Vector3 cameraDelta = m_playerMeshRenderer->GetPosition() - m_sceneCamera->GetPosition();
	math::Vector3 position = (m_playerMeshRenderer->GetRotation() * math::Vector3(0, 0, -forward)) + math::Vector3{ m_playerMeshRenderer->GetPosition().GetX(),m_playerMeshRenderer->GetPosition().GetY(), m_playerMeshRenderer->GetPosition().GetZ() };
	m_playerMeshRenderer->SetPosition(position);

	math::Vector3 cameraPosition = m_playerMeshRenderer->GetPosition() - cameraDelta;

	m_sceneCamera->SetPosition(cameraPosition);

	float mouseX = 0;
	float mouseY = 0;

	if (Input::IsPressed(Input::KeyCode::MouseRigth))
	{
		mouseX = Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseX);
		mouseY = Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseY);
	}

	//// don't apply momentum to mouse inputs
	float rotation = m_RotationSpeed * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(Input::KeyCode::Key_d) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_a) ? -deltaT : 0.0f)
		);

	m_LastForward = forward;

	m_sceneCamera->Update();

	m_CurrentHeading -= rotation;
	if (m_CurrentHeading > DirectX::XM_PI)
		m_CurrentHeading -= DirectX::XM_2PI;
	else if (m_CurrentHeading <= -DirectX::XM_PI)
		m_CurrentHeading += DirectX::XM_2PI;

	math::Matrix3 orientation = math::Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * math::Matrix3::MakeYRotation(m_CurrentHeading);
	m_playerMeshRenderer->SetRotation(math::Quaternion{ orientation });
}

void PlayerController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	float blendedValue;
	if (math::Abs(newValue) > math::Abs(oldValue))
		blendedValue = math::Lerp(newValue, oldValue, math::Pow(0.6f, deltaTime * 60.0f));
	else
		blendedValue = math::Lerp(newValue, oldValue, math::Pow(0.8f, deltaTime * 60.0f));
	oldValue = blendedValue;
	newValue = blendedValue;
}