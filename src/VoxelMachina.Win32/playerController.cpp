#include "pch.h"
#include "playerController.h"
#include "input.h"

PlayerController::PlayerController(math::Vector3 worldUp,
	graphics::MeshRenderer& meshRenderer,
	Camera& sceneCamera) : m_playerMeshRenderer{ meshRenderer }, m_sceneCamera{ sceneCamera }
{
}

void PlayerController::Update(float deltaT)
{
	float forward = m_MoveSpeed * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickY) +
		(Input::IsPressed(Input::KeyCode::Key_s) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_w) ? -deltaT : 0.0f)
		);

	float strafe = m_StrafeSpeed * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftStickX) +
		(Input::IsPressed(Input::KeyCode::Key_d) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_a) ? -deltaT : 0.0f)
		);

	float ascent = m_StrafeSpeed * (
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogRightTrigger) -
		Input::GetTimeCorrectedAnalogInput(Input::AnalogInput::kAnalogLeftTrigger) +
		(Input::IsPressed(Input::KeyCode::Key_e) ? deltaT : 0.0f) +
		(Input::IsPressed(Input::KeyCode::Key_q) ? -deltaT : 0.0f)
		);

	math::Vector3 position = math::Vector3(strafe, ascent, -forward) + m_playerMeshRenderer.GetPosition();

	m_playerMeshRenderer.SetPosition(position);

	m_sceneCamera.SetEyeAtUp(m_sceneCamera.GetPosition(), position, m_WorldUp);

	m_LastForward = forward;
	m_LastStrafe = strafe;
	m_LastAscent = ascent;
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