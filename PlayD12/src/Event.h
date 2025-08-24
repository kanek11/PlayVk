#pragma once
#include "PCH.h"

enum class KeyCode : uint16_t {
    Unknown = 0,

    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Enter, Escape, Tab, Backspace,
    Left, Right, Up, Down,

    MAX_COUNT,
};

constexpr size_t MAX_KeyCode = static_cast<size_t>(KeyCode::MAX_COUNT);


 
enum class MouseButtonCode : uint16_t
{
    // From glfw3.h
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Button6 = 6,
    Button7 = 7,

    ButtonLast = Button7,
    ButtonLeft = Button0,
    ButtonRight = Button1,
    ButtonMiddle = Button2,

    MAX_COUNT
};

constexpr size_t MAX_MouseCode = static_cast<size_t>(MouseButtonCode::MAX_COUNT);


struct FWindowResize
{
    uint32_t width, height;
};

struct FWindowClose
{
};

struct FKeyDown
{
    KeyCode key;
};

struct FKeyUp
{
    KeyCode key;
};


struct FMouseMove { int x, y; };
struct FMouseButtonDown { MouseButtonCode button; int x, y; };
struct FMouseButtonUp { MouseButtonCode button; int x, y; };




using WindowEvent = std::variant<FWindowResize, FWindowClose>;
using InputEvent = std::variant<FKeyDown, FKeyUp, FMouseButtonDown, FMouseButtonUp, FMouseMove>;


struct UIEventBase
{ 
    int x, y;
    bool handled = false;
    void StopPropagation() { handled = true; }
};

struct UIMouseMove : public UIEventBase
{
	UIMouseMove(int xPos, int yPos)
		: UIEventBase{ xPos, yPos } {
	}
};

struct UIMouseButtonDown : public UIEventBase
{
    MouseButtonCode button; 

	UIMouseButtonDown(MouseButtonCode btn, int xPos, int yPos)
		: button(btn), UIEventBase{ xPos, yPos, false } {
	} 
};

struct UIMouseButtonUp : public UIEventBase
{
    MouseButtonCode button; 

	UIMouseButtonUp(MouseButtonCode btn, int xPos, int yPos)
		: button(btn), UIEventBase{ xPos, yPos, false } {
	}
};
 
using UIEvent = std::variant<UIMouseMove, UIMouseButtonDown, UIMouseButtonUp>;


class InputEventQueue {
public:
    static InputEventQueue& Get() {
        static InputEventQueue instance;
        return instance;
    }

    void Push(const InputEvent& event) {
        //std::lock_guard<std::mutex> lock(mutex);
        events.push_back(event);
    }

    std::vector<InputEvent> Drain() {
        //std::lock_guard<std::mutex> lock(mutex);
        std::vector<InputEvent> out = std::move(events);
        events.clear();
        return out;
    }

private:
    std::vector<InputEvent> events;
    //std::mutex mutex;
};



class UIEventQueue  {
public:
    static UIEventQueue& Get() {
        static UIEventQueue instance;
        return instance;
    }

    void Push(const UIEvent& event) {
        //std::lock_guard<std::mutex> lock(mutex);
        events.push_back(event);
    }

    std::vector<UIEvent> Drain() {
        //std::lock_guard<std::mutex> lock(mutex);
        std::vector<UIEvent> out = std::move(events);
        events.clear();
        return out;
    }

private:
    std::vector<UIEvent> events;
    //std::mutex mutex;
};