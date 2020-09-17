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
	m_RotationSpeed = 10.0f;
	m_mouseCameraRotationSpeed = 10.0f;
	m_xboxCamRotationSpeed = 1.5f;

	m_MouseSensitivityX = 0.1f;
	m_MouseSensitivityY = 0.05f;

	auto foward = -math::Matrix3{ m_sceneCamera->GetRotation() }.GetX();

	math::Vector3 forward = Normalize(Cross(m_WorldUp, foward));
	m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

	m_cameraOffset = m_sceneCamera->GetPosition() - m_playerMeshRenderer->GetPosition();

	m_lastCameraRotationX = 0.0f;
	m_lastCameraRotationY = 0.0f;

	m_lastPlayerFoward = 0.0f;
	m_lastPlayerFowardX = 0.0f;
}

void PlayerController::Update(float deltaT)
{
	float forward = m_MoveSpeed * (
		-Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(Input::KeyCode::Key_s) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_w) ? -deltaT : 0.0f)
		);

	float forwardX = m_MoveSpeed * (
		-Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(Input::KeyCode::Key_a) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_d) ? -deltaT : 0.0f)
		);

	float mouseX = 0;
	float mouseY = 0;

	ApplyMomentum(m_lastPlayerFoward, forward, deltaT);
	ApplyMomentum(m_lastPlayerFowardX, forwardX, deltaT);

	if (Input::IsPressed(Input::KeyCode::MouseRigth))
	{
		mouseX = deltaT * (Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseX));
		mouseY = deltaT * (Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseY)) * 0.2f;
	}

	float cameraRotationX = (mouseX * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * deltaT * Input::GetAnalogInput(Input::AnalogInput::kAnalogRightStickX));
	float cameraRotationY = (mouseY * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * deltaT * Input::GetAnalogInput(Input::AnalogInput::kAnalogRightStickY));

	ApplyMomentum(m_lastCameraRotationX, cameraRotationX, deltaT);
	ApplyMomentum(m_lastCameraRotationY, cameraRotationY, deltaT);

	math::Vector3 motionVector = m_sceneCamera->GetRotation() * math::Vector3(-forwardX, 0, -forward);
	motionVector.SetY(0.0f);

	math::Vector3 position = m_playerMeshRenderer->GetPosition() + motionVector;

	m_playerMeshRenderer->SetPosition(position);

	auto YcameraRotation = math::Quaternion(m_playerMeshRenderer->GetRotation() * math::Vector3(0, 1, 0), cameraRotationX);
	auto ZcameraRotation = math::Quaternion(m_sceneCamera->GetRotation() * math::Vector3(1, 0, 0), cameraRotationY);

	auto totalRotation = YcameraRotation * ZcameraRotation;

	if (math::Length(motionVector) >= 0.1f)
	{
		math::Quaternion snewRotation{ math::Matrix3{DirectX::XMMatrixLookToLH(math::Vector3{0, 0, 0}, math::Normalize(motionVector), m_WorldUp)} };
		m_playerMeshRenderer->SetRotation(~snewRotation);
	}

	auto oldCameraOffset = m_cameraOffset;

	m_cameraOffset = (totalRotation * m_cameraOffset);

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

	m_cameraOffset = m_cameraOffset * (1 + 0.01 * deltaT * Input::GetAnalogInput(Input::AnalogInput::kAnalogMouseScroll));
	cameraPosition = m_playerMeshRenderer->GetPosition() + m_cameraOffset;
	m_sceneCamera->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
	m_sceneCamera->Update();

	m_lastCameraRotationX = cameraRotationX;
	m_lastCameraRotationY = cameraRotationY;
	m_lastPlayerFoward = forward;
	m_lastPlayerFowardX = forwardX;
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