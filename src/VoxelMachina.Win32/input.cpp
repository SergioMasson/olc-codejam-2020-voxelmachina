#include "pch.h"
#include "input.h"

//use XInput to handle input.
#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")

static bool s_Buttons[2][(int)Input::KeyCode::NumDigitalInputs];
static float s_HoldDuration[(int)Input::KeyCode::NumDigitalInputs] = { 0.0f };
static float s_Analogs[static_cast<uint32_t>(Input::AnalogInput::kNumAnalogInputs)];
static float s_AnalogsTC[static_cast<uint32_t>(Input::AnalogInput::kNumAnalogInputs)];

float FilterAnalogInput(int val, int deadZone)
{
	if (val < 0)
	{
		if (val > -deadZone)
			return 0.0f;
		else
			return (val + deadZone) / (32768.0f - deadZone);
	}
	else
	{
		if (val < deadZone)
			return 0.0f;
		else
			return (val - deadZone) / (32767.0f - deadZone);
	}
}

void Input::Initialize()
{
	ZeroMemory(s_Buttons, sizeof(s_Buttons));
	ZeroMemory(s_Analogs, sizeof(s_Analogs));
}

void Input::Shutdown()
{
}

void Input::Update(float frameDelta)
{
	memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));

	//Cleans up buffers.
	memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
	memset(s_Analogs, 0, sizeof(s_Analogs));

	XINPUT_STATE newInputState;

	//Get the values for all controller buttons from XINPUT
	if (ERROR_SUCCESS == XInputGetState(0, &newInputState))
	{
		if (newInputState.Gamepad.wButtons & (1 << 0)) s_Buttons[0][static_cast<uint32_t>(KeyCode::DPadUp)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 1)) s_Buttons[0][static_cast<uint32_t>(KeyCode::DPadDown)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 2)) s_Buttons[0][static_cast<uint32_t>(KeyCode::DPadLeft)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 3)) s_Buttons[0][static_cast<uint32_t>(KeyCode::DPadRight)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 4)) s_Buttons[0][static_cast<uint32_t>(KeyCode::StartButton)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 5)) s_Buttons[0][static_cast<uint32_t>(KeyCode::BackButton)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 6)) s_Buttons[0][static_cast<uint32_t>(KeyCode::LThumbClick)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 7)) s_Buttons[0][static_cast<uint32_t>(KeyCode::RThumbClick)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 8)) s_Buttons[0][static_cast<uint32_t>(KeyCode::LShoulder)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 9)) s_Buttons[0][static_cast<uint32_t>(KeyCode::RShoulder)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 12)) s_Buttons[0][static_cast<uint32_t>(KeyCode::AButton)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 13)) s_Buttons[0][static_cast<uint32_t>(KeyCode::BButton)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 14)) s_Buttons[0][static_cast<uint32_t>(KeyCode::XButton)] = true;
		if (newInputState.Gamepad.wButtons & (1 << 15)) s_Buttons[0][static_cast<uint32_t>(KeyCode::YButton)] = true;

		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogLeftTrigger)] = newInputState.Gamepad.bLeftTrigger / 255.0f;
		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogRightTrigger)] = newInputState.Gamepad.bRightTrigger / 255.0f;
		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogLeftStickX)] = FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogLeftStickY)] = FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogRightStickX)] = FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogRightStickY)] = FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}

	// Update time duration for buttons pressed
	for (uint32_t i = 0; i < (uint32_t)KeyCode::NumDigitalInputs; ++i)
	{
		if (s_Buttons[0][i])
		{
			if (!s_Buttons[1][i])
				s_HoldDuration[i] = 0.0f;
			else
				s_HoldDuration[i] += frameDelta;
		}
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(AnalogInput::kNumAnalogInputs); ++i)
	{
		s_AnalogsTC[i] = s_Analogs[i] * frameDelta;
	}
}

bool Input::IsAnyPressed(void)
{
	return s_Buttons[0] != 0;
}

bool Input::IsPressed(KeyCode di)
{
	return s_Buttons[0][static_cast<uint32_t>(di)];
}

bool Input::IsFirstPressed(KeyCode di)
{
	return s_Buttons[0][static_cast<uint32_t>(di)] && !s_Buttons[1][static_cast<uint32_t>(di)];
}

bool Input::IsReleased(KeyCode di)
{
	return !s_Buttons[0][static_cast<uint32_t>(di)];
}

bool Input::IsFirstReleased(KeyCode di)
{
	return !s_Buttons[0][static_cast<uint32_t>(di)] && s_Buttons[1][static_cast<uint32_t>(di)];
}

float Input::GetDurationPressed(KeyCode di)
{
	return s_HoldDuration[static_cast<uint32_t>(di)];
}

float Input::GetAnalogInput(AnalogInput ai)
{
	return s_Analogs[static_cast<uint32_t>(ai)];
}

float Input::GetTimeCorrectedAnalogInput(AnalogInput ai)
{
	return s_AnalogsTC[static_cast<uint32_t>(ai)];
}