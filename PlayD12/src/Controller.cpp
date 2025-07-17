#include "PCH.h"

#include "Controller.h"


void GamepadInput::Update()
{
    for (DWORD i = 0; i < m_states.size(); ++i) {
        XINPUT_STATE state{};
        DWORD result = XInputGetState(i, &state);
        auto& pad = m_states[i];

		pad.connected = (result == ERROR_SUCCESS);
        if (!pad.connected)
            continue;

        FillState(pad, state.Gamepad);
    }
}

const GamepadState& GamepadInput::GetState(int i) const
{
    static GamepadState dummy{};
    return (i >= 0 && i < m_states.size()) ? m_states[i] : dummy;
}

void GamepadInput::FillState(GamepadState& pad, const XINPUT_GAMEPAD& g)
{
    using enum GamepadButton;

    pad.buttons.reset();
    pad.buttons[(size_t)A] = (g.wButtons & XINPUT_GAMEPAD_A);
    pad.buttons[(size_t)B] = (g.wButtons & XINPUT_GAMEPAD_B);
    pad.buttons[(size_t)X] = (g.wButtons & XINPUT_GAMEPAD_X);
    pad.buttons[(size_t)Y] = (g.wButtons & XINPUT_GAMEPAD_Y); 


    pad.buttons[(size_t)DPadUp] = (g.wButtons & XINPUT_GAMEPAD_DPAD_UP);
    pad.buttons[(size_t)DPadDown] = (g.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
    pad.buttons[(size_t)DPadLeft] = (g.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
    pad.buttons[(size_t)DPadRight] = (g.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);


    pad.buttons[(size_t)LeftShoulder] = (g.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
    pad.buttons[(size_t)RightShoulder] = (g.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);


    pad.buttons[(size_t)Back] = (g.wButtons & XINPUT_GAMEPAD_BACK);
    pad.buttons[(size_t)Start] = (g.wButtons & XINPUT_GAMEPAD_START);
    pad.buttons[(size_t)LeftThumb] = (g.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
    pad.buttons[(size_t)RightThumb] = (g.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);


	pad.axes[static_cast<size_t>(GamepadAxis::LX)] = (std::abs(g.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ? 0.f : g.sThumbLX / 32767.0f;
	pad.axes[static_cast<size_t>(GamepadAxis::LY)] = (std::abs(g.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ? 0.f : g.sThumbLY / 32767.0f;
	pad.axes[static_cast<size_t>(GamepadAxis::RX)] = (std::abs(g.sThumbRX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ? 0.f : g.sThumbRX / 32767.0f;
	pad.axes[static_cast<size_t>(GamepadAxis::RY)] = (std::abs(g.sThumbRY) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ? 0.f : g.sThumbRY / 32767.0f;


	pad.axes[static_cast<size_t>(GamepadAxis::LT)] = (g.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0.f : g.bLeftTrigger / 255.0f;
	pad.axes[static_cast<size_t>(GamepadAxis::RT)] = (g.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0.f : g.bRightTrigger / 255.0f;

}
