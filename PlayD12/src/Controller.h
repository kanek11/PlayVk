#pragma once

#include "PCH.h"

#include "Xinput.h" 
#pragma comment(lib, "Xinput9_1_0.lib")

//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
//#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689


enum class GamepadButton : int {
    A, B, X, Y,
    LeftShoulder, RightShoulder,
    Back, Start,
    LeftThumb, RightThumb,
    DPadUp, DPadDown, DPadLeft, DPadRight,
    COUNT
};
static constexpr int ButtonCount = static_cast<int>(GamepadButton::COUNT);


struct GamepadState {
    bool connected = false;
    std::array<bool, ButtonCount> buttons{};
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;
    float thumbLX = 0.0f;
    float thumbLY = 0.0f;
    float thumbRX = 0.0f;
    float thumbRY = 0.0f;
};

class GamepadInput {
public:
    void Update();
    const GamepadState& GetState(int index) const { return m_states[index]; }

private:
    GamepadState m_states[4]; //up to 4
};


void GamepadInput::Update() {
    for (DWORD i = 0; i < 4; ++i) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        DWORD result = XInputGetState(i, &state);
        GamepadState& gp = m_states[i];
        gp.connected = (result == ERROR_SUCCESS);

        if (!gp.connected) continue;

        const auto& g = state.Gamepad;

        //gp.buttons[0] = g.wButtons & XINPUT_GAMEPAD_A;
        //gp.buttons[1] = g.wButtons & XINPUT_GAMEPAD_B;
        // ... others
        gp.buttons[(int)GamepadButton::A] = g.wButtons & XINPUT_GAMEPAD_A;
        gp.buttons[(int)GamepadButton::B] = g.wButtons & XINPUT_GAMEPAD_B;


        gp.leftTrigger = g.bLeftTrigger / 255.0f;
        gp.rightTrigger = g.bRightTrigger / 255.0f;

        gp.thumbLX = g.sThumbLX / 32767.0f;
        gp.thumbLY = g.sThumbLY / 32767.0f;
        gp.thumbRX = g.sThumbRX / 32767.0f;
        gp.thumbRY = g.sThumbRY / 32767.0f;
    }
}