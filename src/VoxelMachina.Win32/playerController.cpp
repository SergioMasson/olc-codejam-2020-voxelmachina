#include "pch.h"
#include "playerController.h"
#include "input.h"

PlayerController::PlayerController(math::Vector3 worldUp,
	graphics::MeshRenderer* meshRenderer,
	Camera* sceneCamera,
	bool lockCamera) :
	m_playerMeshRenderer{ meshRenderer },
	m_sceneCamera{ sceneCamera },
	m_lockCamera{ lockCamera }
{
	m_WorldUp = Normalize(worldUp);
	m_WorldNorth = Normalize(Cross(m_WorldUp, math::Vector3(1, 0, 0)));
	m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

	m_MoveSpeed = 10.0f;
	m_RotationSpeed = 2.0f;
	m_mouseCameraRotationSpeed = 10.0f;
	m_xboxCamRotationSpeed = 1.0f;

	m_MouseSensitivityX = 0.1f;
	m_MouseSensitivityY = 0.05f;

	auto foward = -math::Matrix3{ m_sceneCamera->GetRotation() }.GetX();

	math::Vector3 forward = Normalize(Cross(m_WorldUp, foward));
	m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

	m_cameraOffset = m_sceneCamera->GetPosition() - m_playerMeshRenderer->GetPosition();

	m_lastCameraRotationX = 0.0f;
	m_lastCameraRotationY = 0.0f;

	m_lastPlayerRotation = 0.0f;
	m_lastPlayerFoward = 0.0f;
}

void PlayerController::Update(float deltaT)
{
	float forward = m_MoveSpeed * (
		-Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(Input::KeyCode::Key_s) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_w) ? -deltaT : 0.0f)
		);

	float mouseX = 0;
	float mouseY = 0;

	ApplyMomentum(m_lastPlayerFoward, forward, deltaT);

	if (Input::IsPressed(Input::KeyCode::MouseRigth))
	{
		mouseX = deltaT * (Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseX));
		mouseY = deltaT * (Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseY)) * 0.2f;
	}

	float cameraRotationX = (mouseX * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * deltaT * Input::GetAnalogInput(Input::AnalogInput::kAnalogRightStickX));
	float cameraRotationY = (mouseY * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * 0.2 * deltaT * Input::GetAnalogInput(Input::AnalogInput::kAnalogRightStickY));

	ApplyMomentum(m_lastCameraRotationX, cameraRotationX, deltaT);
	ApplyMomentum(m_lastCameraRotationY, cameraRotationY, deltaT);

	//// don't apply momentum to mouse inputs
	float rotation = m_RotationSpeed * (
		-Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(Input::KeyCode::Key_d) ? -deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_a) ? deltaT : 0.0f)
		);

	ApplyMomentum(m_lastPlayerRotation, rotation, deltaT);

	math::Vector3 position = (m_playerMeshRenderer->GetRotation() * math::Vector3(0, 0, -forward)) + math::Vector3{ m_playerMeshRenderer->GetPosition().GetX(),m_playerMeshRenderer->GetPosition().GetY(), m_playerMeshRenderer->GetPosition().GetZ() };
	m_playerMeshRenderer->SetPosition(position);

	auto YcameraRotation = math::Quaternion(m_playerMeshRenderer->GetRotation() * math::Vector3(0, 1, 0), cameraRotationX);
	auto ZcameraRotation = math::Quaternion(m_sceneCamera->GetRotation() * math::Vector3(1, 0, 0), cameraRotationY);

	auto totalRotation = YcameraRotation * ZcameraRotation;

	m_CurrentHeading -= rotation;
	if (m_CurrentHeading > DirectX::XM_PI)
		m_CurrentHeading -= DirectX::XM_2PI;
	else if (m_CurrentHeading <= -DirectX::XM_PI)
		m_CurrentHeading += DirectX::XM_2PI;

	math::Matrix3 orientation = math::Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * math::Matrix3::MakeYRotation(m_CurrentHeading);

	m_playerMeshRenderer->SetRotation(math::Quaternion{ orientation });

	auto oldCameraOffset = m_cameraOffset;

	if (m_lockCamera)
	{
		m_cameraOffset = math::Matrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * math::Matrix3::MakeYRotation(-rotation) * (totalRotation * m_cameraOffset);
	}
	else
	{
		m_cameraOffset = (totalRotation * m_cameraOffset);
	}

	math::Vector3 cameraPosition = m_playerMeshRenderer->GetPosition() + m_cameraOffset;

	m_sceneCamera->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
	m_sceneCamera->Update();

	float cameraFowardDot = math::Dot(m_sceneCamera->GetForwardVec(), m_WorldUp);

	if (cameraFowardDot <= 0.1f || cameraFowardDot > 0.80f)
	{
		m_cameraOffset = oldCameraOffset;
		m_sceneCamera->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
		m_sceneCamera->Update();
	}

	m_lastCameraRotationX = cameraRotationX;
	m_lastCameraRotationY = cameraRotationY;
	m_lastPlayerFoward = forward;
	m_lastPlayerRotation = rotation;
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