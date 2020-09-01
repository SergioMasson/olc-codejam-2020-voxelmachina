#include "pch.h"
#include "input.h"
#include "Winuser.h"
#include <map>
#include "graphics/coreGraphics.h"

//use XInput to handle input.
#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")

//Buffer for all buttons.
static bool s_Buttons[2][(int)Input::KeyCode::NumDigitalInputs];

//Buffer for all buttons hold durations.
static float s_HoldDuration[(int)Input::KeyCode::NumDigitalInputs] = { 0.0f };
static float s_Analogs[static_cast<uint32_t>(Input::AnalogInput::kNumAnalogInputs)];
static float s_AnalogsTC[static_cast<uint32_t>(Input::AnalogInput::kNumAnalogInputs)];

static std::map<WPARAM, Input::KeyCode> windowToKeyMap;

void KbmBuildKeyMapping()
{
	//Maps the WPARAM to their keycodes. See (https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
	windowToKeyMap = std::map<WPARAM, Input::KeyCode>
	{
		{0x1B,Input::KeyCode::Key_escape },
		{0x30,Input::KeyCode::Key_0 },
		{0x31,Input::KeyCode::Key_1 },
		{0x32,Input::KeyCode::Key_2 },
		{0x33,Input::KeyCode::Key_3 },
		{0x34,Input::KeyCode::Key_4 },
		{0x35,Input::KeyCode::Key_5 },
		{0x36,Input::KeyCode::Key_6 },
		{0x37,Input::KeyCode::Key_7 },
		{0x38,Input::KeyCode::Key_8 },
		{0x39,Input::KeyCode::Key_9 },
		{0x41,Input::KeyCode::Key_a },
		{0x42,Input::KeyCode::Key_b },
		{0x43,Input::KeyCode::Key_c },
		{0x44,Input::KeyCode::Key_d },
		{0x45,Input::KeyCode::Key_e },
		{0x46,Input::KeyCode::Key_f },
		{0x47,Input::KeyCode::Key_g },
		{0x48,Input::KeyCode::Key_h },
		{0x49,Input::KeyCode::Key_i },
		{0x4A,Input::KeyCode::Key_j },
		{0x4B,Input::KeyCode::Key_k },
		{0x4C,Input::KeyCode::Key_l },
		{0x4D,Input::KeyCode::Key_m },
		{0x4E,Input::KeyCode::Key_n },
		{0x4F,Input::KeyCode::Key_o },
		{0x50,Input::KeyCode::Key_p },
		{0x51,Input::KeyCode::Key_q },
		{0x52,Input::KeyCode::Key_r },
		{0x53,Input::KeyCode::Key_s },
		{0x54,Input::KeyCode::Key_t },
		{0x55,Input::KeyCode::Key_u },
		{0x56,Input::KeyCode::Key_v },
		{0x57,Input::KeyCode::Key_w },
		{0x58,Input::KeyCode::Key_x },
		{0x59,Input::KeyCode::Key_x },
		{0x60,Input::KeyCode::Key_z },

		//Non alpha numeric keys
		{VK_SPACE, Input::KeyCode::Key_space},
		{VK_UP, Input::KeyCode::Key_up},
		{VK_DOWN, Input::KeyCode::Key_down},
		{VK_LEFT, Input::KeyCode::Key_left},
		{VK_RIGHT, Input::KeyCode::Key_right},
		{VK_SPACE, Input::KeyCode::Key_space},
		{VK_RETURN, Input::KeyCode::Key_return},
		{VK_ADD, Input::KeyCode::Key_add},
		{VK_SUBTRACT, Input::KeyCode::Key_minus},

		//Mouse buttons
		{VK_RBUTTON, Input::KeyCode::MouseRigth},
		{VK_LBUTTON, Input::KeyCode::MouseLeft},
		{VK_MBUTTON, Input::KeyCode::MouseMiddle},

		//Numpad
		{VK_NUMPAD0, Input::KeyCode::Key_numpad0},
		{VK_NUMPAD1, Input::KeyCode::Key_numpad1},
		{VK_NUMPAD2, Input::KeyCode::Key_numpad2},
		{VK_NUMPAD3, Input::KeyCode::Key_numpad3},
		{VK_NUMPAD4, Input::KeyCode::Key_numpad4},
		{VK_NUMPAD5, Input::KeyCode::Key_numpad5},
		{VK_NUMPAD6, Input::KeyCode::Key_numpad6},
		{VK_NUMPAD7, Input::KeyCode::Key_numpad7},
		{VK_NUMPAD8, Input::KeyCode::Key_numpad8},
		{VK_NUMPAD9, Input::KeyCode::Key_numpad9},

		//F keys
		{VK_F1, Input::KeyCode::Key_f1},
		{VK_F2, Input::KeyCode::Key_f2},
		{VK_F3, Input::KeyCode::Key_f3},
		{VK_F4, Input::KeyCode::Key_f4},
		{VK_F5, Input::KeyCode::Key_f5},
		{VK_F6, Input::KeyCode::Key_f6},
		{VK_F7, Input::KeyCode::Key_f7},
		{VK_F8, Input::KeyCode::Key_f8},
		{VK_F9, Input::KeyCode::Key_f9},
		{VK_F10, Input::KeyCode::Key_f10},
		{VK_F11, Input::KeyCode::Key_f11},
		{VK_F12, Input::KeyCode::Key_f12}
	};
}

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

	KbmBuildKeyMapping();
}

void Input::Shutdown()
{
}

//Reads all XINPUT values and updated the input table.
void Input::Update(float frameDelta)
{
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

//Cleans up everthing for the next frame.
void Input::PostUpdate()
{
	memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));

	//Cleans up buffers.
	memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
	memset(s_Analogs, 0, sizeof(s_Analogs));
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

void Input::SetKey(WPARAM key, bool isDown)
{
	auto keyFind = windowToKeyMap.find(key);

	if (keyFind != windowToKeyMap.end())
	{
		auto targetKey = windowToKeyMap[key];
		s_Buttons[0][static_cast<uint32_t>(targetKey)] = true;
	}
}

void Input::UpdateMousePosition(float x, float y)
{
	float normalizedX = (x / graphics::g_windowWidth) - 0.5;
	float normalizedY = (y / graphics::g_windowHeight) - 0.5;

	s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogMouseX)] = -normalizedX;
	s_Analogs[static_cast<uint32_t>(AnalogInput::kAnalogMouseY)] = normalizedY;
}