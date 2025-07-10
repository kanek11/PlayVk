#pragma once 
#include "PCH.h"

#include "Base.h"
#include "Event.h"

struct BoolEdgeDetector {
    bool previous = false;
    bool risingEdge = false;
    bool fallingEdge = false;

    void Update(bool current) {
        risingEdge = (!previous && current);
        fallingEdge = (previous && !current);
        previous = current;
    }

    bool Get() const { return previous; }
    operator bool() const { return previous; }
};
 

enum class ButtonFrameState {
	Down,
	Up,
	Pressed, // just pressed
	Released // just released
};

struct ButtonFrame  { 
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

private:
    std::array<ButtonFrame, MaxKeyCode> keyState;
    std::array<bool, MaxKeyCode> keyStateRaw;
    //SharedPtr<IInputSource> inputSource; 
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

    //InputSystem* inputSystem;
    //static std::unordered_map<int, KeyCode> windowsKeyMap;
};