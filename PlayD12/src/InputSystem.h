#pragma once 
#include "PCH.h"

#include "Base.h"
#include "Event.h"

#include "Controller.h"

#include "Math/GenericUtils.h"

enum class EAxis { MoveX, MoveY, MoveZ, COUNT };
constexpr size_t MAX_Axis = static_cast<size_t>(EAxis::COUNT);

enum class EAction {
	Jump, Normal, Metal, Ice, Clone,
	NavigateUp, NavigateDown,
	Confirm, Cancel, 
	Pause,
	COUNT
};

struct Binding {
	KeyCode key;

	GamepadButton gamepadButton{ GamepadButton::COUNT };
	float buttonScale = 1.0f;

	GamepadAxis gamepadAxis{ GamepadAxis::COUNT }; //invalid;
};

inline std::unordered_map<EAxis, std::vector<Binding>> DefaultAxisBindings = { 
	{
		EAxis::MoveX,
		{
			Binding{.key = KeyCode::A, .buttonScale = -1.0f },
			Binding{.key = KeyCode::D, .buttonScale = 1.0f },
			Binding {.key = KeyCode::Left, .buttonScale = -1.0f },
			Binding {.key = KeyCode::Right, .buttonScale = 1.0f },

			Binding {.gamepadButton = GamepadButton::DPadLeft, .buttonScale = -1.0f },
			Binding {.gamepadButton = GamepadButton::DPadRight, .buttonScale = 1.0f },
			Binding { .gamepadAxis = GamepadAxis::LX },
					//{ GamepadAxis::RX },  

		},
	},

	 {
		 EAxis::MoveZ,
		 {
			 Binding{.key = KeyCode::W, .buttonScale = 1.0f },
			 Binding{.key = KeyCode::S, .buttonScale = -1.0f },
			 Binding{.key = KeyCode::Up, .buttonScale = 1.0f },
			 Binding{.key = KeyCode::Down, .buttonScale = -1.0f },

			 Binding{.gamepadButton = GamepadButton::DPadUp, .buttonScale = 1.0f },
			 Binding{.gamepadButton = GamepadButton::DPadDown, .buttonScale = -1.0f },
			 Binding {.gamepadAxis = GamepadAxis::LY },
			 //{ GamepadAxis::RY },
		 },
	 },

};


inline std::unordered_map<EAction, std::vector<Binding>> DefaultActionBindings = {
	{
		EAction::Jump,
		{
			Binding{.key = KeyCode::Space },
			Binding{.gamepadButton = GamepadButton::RightShoulder,  },
		},
	},
	{
		EAction::Normal,
		{
			Binding{.key = KeyCode::Num1 },
			Binding{.gamepadButton = GamepadButton::A,  },
		},
	},
	{
		EAction::Metal,
		{
			Binding{.key = KeyCode::Num2 },
			Binding{.gamepadButton = GamepadButton::X,  },
		},
	},
	{
		EAction::Ice,
		{
			Binding{.key = KeyCode::Num3 },
			Binding{.gamepadButton = GamepadButton::Y, },
		},
	},
	{
		EAction::Clone,
		{
			Binding{.key = KeyCode::Num4 },
			Binding{.gamepadButton = GamepadButton::B, },
		},
	},

	{
		EAction::NavigateUp,
		{
			Binding{.key = KeyCode::W, .buttonScale = 1.0f },
			Binding{.key = KeyCode::Up, .buttonScale = 1.0f },
			Binding{.gamepadButton = GamepadButton::DPadUp, .buttonScale = 1.0f },
		},
	},
	{
		EAction::NavigateDown,
		{
			Binding{.key = KeyCode::S, .buttonScale = -1.0f },
			Binding{.key = KeyCode::Down, .buttonScale = -1.0f },
			Binding{.gamepadButton = GamepadButton::DPadDown, .buttonScale = -1.0f },
		},
	},

	{
		EAction::Confirm,
		{
			Binding{.key = KeyCode::Enter },
			Binding{.gamepadButton = GamepadButton::A,  },
		},
	},
	{
		EAction::Cancel,
		{
			Binding{.key = KeyCode::Escape },
			Binding{.gamepadButton = GamepadButton::B,  },
		},
	},

	{
		EAction::Pause,
		{
			Binding{.key = KeyCode::Escape },
			Binding{.gamepadButton = GamepadButton::Start,  },
		},
	},

};


struct FPointer
{
	int x, y;
};


enum class ButtonFrameState {
	Down,
	Up,
	Pressed, // just pressed
	Released // just released
};

struct ButtonFrame {

	void Update(bool pressed) {
		edgeDetector.Update(pressed);
		if (edgeDetector.risingEdge) {
			state = ButtonFrameState::Pressed;
			//std::cout << "KeyFrame state updated: " << static_cast<int>(state) << '\n';
		}
		else if (edgeDetector.fallingEdge) {
			state = ButtonFrameState::Released;
			//std::cout << "KeyFrame state updated: " << static_cast<int>(state) << '\n';
		}
		else {
			state = edgeDetector.Get() ? ButtonFrameState::Down : ButtonFrameState::Up;
			//std::cout << "KeyFrame state updated: " << static_cast<int>(state) << '\n';
		}
	}
	ButtonFrameState state = ButtonFrameState::Up;
private:
	BoolEdgeDetector edgeDetector;
};


class InputSystem
{
public:
	InputSystem();

	void OnUpdate();

	void OnKeyDown(KeyCode key);
	void OnKeyUp(KeyCode key);

	bool IsKeyJustPressed(KeyCode key) const;
	bool IsKeyJustReleased(KeyCode key) const;
	bool IsKeyDown(KeyCode key) const;
	bool IsKeyUp(KeyCode key) const;


	void OnMouseButtonDown(FMouseButtonDown e);
	void OnMouseButtonUp(FMouseButtonUp e);
	void OnMouseMove(FMouseMove e);

	FPointer GetMousePointer();

	bool IsMouseButtonJustPressed(MouseButtonCode button);



private:
	std::array<ButtonFrame, MAX_KeyCode> keyState;
	std::array<bool, MAX_KeyCode> keyStateRaw;
	//SharedPtr<IInputSource> inputSource; 

	std::array<ButtonFrame, MAX_MouseCode> mouseButtonState;
	std::array<bool, MAX_MouseCode> mouseButtonStateRaw;
	int MouseX, MouseY;


	//new:controller:
	GamepadInput m_gamepad;

	//new:
public:
	float GetAxis(EAxis axis) const;
	std::array<float, (size_t)EAxis::COUNT> m_axisState{};


	bool GetAction(EAction action) const {
		return m_actionState[static_cast<size_t>(action)];
	}
	std::array<bool, (size_t)EAction::COUNT> m_actionState{};
};


struct IInputSource {
	virtual ~IInputSource() = default;
};


struct WindowsInputSource : IInputSource {
public:
	virtual ~WindowsInputSource() = default;
	WindowsInputSource() = default;;
	//WindowsInputSource(InputSystem* input):
	//    inputSystem(input)
	//{}

	void OnKeyUp(int key);

	void OnKeyDown(int key);

	//void OnMouseButtonDown();

	//InputSystem* inputSystem;
	//static std::unordered_map<int, KeyCode> windowsKeyMap;
};