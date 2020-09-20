#include "pch.h"
#include "playerController.h"
#include "input.h"

PlayerController::PlayerController(GameObject* gameObject, math::Vector3 worldUp) : BehaviourComponent(gameObject)
{
	m_WorldUp = Normalize(worldUp);
	m_WorldNorth = Normalize(Cross(m_WorldUp, math::Vector3(1, 0, 0)));
	m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

	m_MoveSpeed = 10.0f;
	m_RotationSpeed = 10.0f;
	m_mouseCameraRotationSpeed = 10.0f;
	m_xboxCamRotationSpeed = 1.5f;
	m_cameraZoomSpeed = 0.01f;

	m_MouseSensitivityX = 0.1f;
	m_MouseSensitivityY = 0.05f;

	auto foward = -math::Matrix3{ Camera::MainCamera()->GetRotation() }.GetX();

	math::Vector3 forward = Normalize(Cross(m_WorldUp, foward));
	m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

	m_cameraOffset = Camera::MainCamera()->GetPosition() - m_gameObject->GetPosition();

	m_lastCameraRotationX = 0.0f;
	m_lastCameraRotationY = 0.0f;

	m_lastPlayerFoward = 0.0f;
	m_lastPlayerFowardX = 0.0f;

	m_lastPlayerFoward = 0.0f;
	m_lastCameraDelta = 0.0f;
}

void PlayerController::Update(float deltaT)
{
	//Update player position
	float forward = m_MoveSpeed * (
		-Input::GetTimeCorrectedAnalogInput(AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(KeyCode::Key_s) ? deltaT : 0.0f) +
		(Input::IsPressed(KeyCode::Key_w) ? -deltaT : 0.0f)
		);

	float forwardX = m_MoveSpeed * (
		-Input::GetTimeCorrectedAnalogInput(AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(KeyCode::Key_a) ? deltaT : 0.0f) +
		(Input::IsPressed(KeyCode::Key_d) ? -deltaT : 0.0f)
		);

	if ((Input::IsPressed(KeyCode::Key_s) || Input::IsPressed(KeyCode::Key_w))
		&& (Input::IsPressed(KeyCode::Key_a) || Input::IsPressed(KeyCode::Key_d)))
	{
		auto tanget = (forward * forward + forwardX * forwardX);
		auto value = sqrt(tanget) / 2.0f;

		forward = forward >= 0 ? value : -value;
		forwardX = forwardX >= 0 ? value : -value;
	}

	math::Vector3 motionVector = Camera::MainCamera()->GetRotation() * math::Vector3(-forwardX, 0, -forward);
	motionVector.SetY(0.0f);

	bool hasBoost = Input::IsPressed(KeyCode::XButton) || Input::IsPressed(KeyCode::Key_space);

	if (hasBoost)
		motionVector = motionVector * 1.5f;

	math::Vector3 position = m_gameObject->GetPosition() + motionVector;
	m_gameObject->SetPosition(position);

	//Update player orientation
	if (math::Length(motionVector) > 0.000001f)
	{
		float size = math::Length(motionVector);
		math::Quaternion snewRotation{ math::Matrix3{DirectX::XMMatrixLookToLH(math::Vector3{0, 0, 0}, math::Normalize(motionVector), m_WorldUp)} };
		snewRotation = math::Lerp(m_gameObject->GetRotation(), ~snewRotation, size);
		m_gameObject->SetRotation(snewRotation);
	}

	//Update camera rotation
	float mouseX = 0;
	float mouseY = 0;

	ApplyMomentum(m_lastPlayerFoward, forward, deltaT);
	ApplyMomentum(m_lastPlayerFowardX, forwardX, deltaT);

	if (Input::IsPressed(KeyCode::MouseRigth))
	{
		mouseX = deltaT * (Input::GetAnalogInput(AnalogInput::kAnalogMouseX));
		mouseY = deltaT * (Input::GetAnalogInput(AnalogInput::kAnalogMouseY)) * 0.2f;
	}

	float cameraRotationX = (mouseX * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * deltaT * Input::GetAnalogInput(AnalogInput::kAnalogRightStickX));
	float cameraRotationY = (mouseY * m_mouseCameraRotationSpeed) + (m_xboxCamRotationSpeed * deltaT * Input::GetAnalogInput(AnalogInput::kAnalogRightStickY));

	ApplyMomentum(m_lastCameraRotationX, cameraRotationX, deltaT);
	ApplyMomentum(m_lastCameraRotationY, cameraRotationY, deltaT);

	auto YcameraRotation = math::Quaternion(m_gameObject->GetRotation() * math::Vector3(0, 1, 0), cameraRotationX);
	auto ZcameraRotation = math::Quaternion(Camera::MainCamera()->GetRotation() * math::Vector3(1, 0, 0), cameraRotationY);

	auto totalRotation = YcameraRotation * ZcameraRotation;

	auto oldCameraOffset = m_cameraOffset;

	m_cameraOffset = (totalRotation * m_cameraOffset);

	math::Vector3 cameraPosition = m_gameObject->GetPosition() + m_cameraOffset;

	Camera::MainCamera()->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
	Camera::MainCamera()->Update();

	float cameraFowardDot = math::Dot(Camera::MainCamera()->GetForwardVec(), m_WorldUp);

	if (cameraFowardDot <= 0.1f || cameraFowardDot > 0.80f)
	{
		m_cameraOffset = oldCameraOffset;
		Camera::MainCamera()->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
		Camera::MainCamera()->Update();
	}

	float zoomDelta = m_cameraZoomSpeed * (Input::GetTimeCorrectedAnalogInput(AnalogInput::kAnalogMouseScroll) +
		(Input::IsPressed(KeyCode::LShoulder) ? deltaT * 75 : 0) +
		(Input::IsPressed(KeyCode::RShoulder) ? -deltaT * 75 : 0));

	ApplyMomentum(m_lastCameraDelta, zoomDelta, deltaT);

	m_cameraOffset = m_cameraOffset * (1 + zoomDelta);
	cameraPosition = m_gameObject->GetPosition() + m_cameraOffset;
	Camera::MainCamera()->SetEyeAtUp(cameraPosition, position, math::Vector3(0, 1, 0));
	Camera::MainCamera()->Update();

	m_lastCameraDelta = zoomDelta;
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