#pragma once
#include <array>
#include <bitset>
#include <cstdint>
#include <xinput.h>     // Windows XInput
#pragma comment(lib, "Xinput9_1_0.lib")

//---------------------- 
enum class GamepadButton : uint8_t {
    A, B, X, Y,
    LeftShoulder, RightShoulder,
    Back, Start,
    LeftThumb, RightThumb,
    DPadUp, DPadDown, DPadLeft, DPadRight,
    COUNT
};

static constexpr size_t ButtonCount = static_cast<size_t>(GamepadButton::COUNT);


enum class GamepadAxis { LX, LY, RX, RY, LT, RT, COUNT };

 
//---------------------- 
struct GamepadState {
    bool connected = false;
    std::bitset<ButtonCount> buttons;
    std::bitset<ButtonCount> prevButtons;   
   
	std::array<float, static_cast<size_t>(GamepadAxis::COUNT)> axes{};

    //just pressed or released:
	bool IsButtonPressed(GamepadButton button) const {
		return buttons.test(static_cast<size_t>(button)) && !prevButtons.test(static_cast<size_t>(button));
	}

	bool IsButtonReleased(GamepadButton button) const {
		return !buttons.test(static_cast<size_t>(button)) && prevButtons.test(static_cast<size_t>(button));
	} 
	bool IsAnyButtonPressed() const {
		return buttons.any();
	} 
	//bool IsAnyButtonReleased() const {
	//	return (buttons ^ prevButtons).any();
	//}
};

//---------------------- 
class GamepadInput {
public:
    void Update();                              
    const GamepadState& GetState(int i) const;   

	bool GetButtonState(GamepadButton button, int i = 0) const {
		if (i < 0 || i >= m_states.size()) return false;
		return m_states[i].buttons.test(static_cast<size_t>(button));
	}

	bool IsButtonPressed(GamepadButton button, int i = 0) const {
		if (i < 0 || i >= m_states.size()) return false;
		return m_states[i].IsButtonPressed(button);
	}

	bool IsButtonReleased(GamepadButton button, int i = 0) const {
		if (i < 0 || i >= m_states.size()) return false;
		return m_states[i].IsButtonReleased(button);
	}
     
	float GetAxis(GamepadAxis axis, int i = 0) const {
		if (i < 0 || i >= m_states.size()) return 0.f;
		return m_states[i].axes[static_cast<size_t>(axis)];
	}

private:
    std::array<GamepadState, 4> m_states;

    // Mapping from enum to XInput
    void FillState(GamepadState& dst, const XINPUT_GAMEPAD& src);
};
