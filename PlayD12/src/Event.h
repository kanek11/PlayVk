#pragma once
#include "PCH.h"

enum class KeyCode : uint16_t {
    Unknown = 0,
    /*    KeyPH = 64,*/
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Enter, Escape, Tab, Backspace,
    Left, Right, Up, Down,

    COUNT,
};

constexpr size_t MaxKeyCode = static_cast<size_t>(KeyCode::COUNT);

struct WindowResize
{
    uint32_t width, height;
};

struct WindowClose
{
};

struct KeyDown
{
    KeyCode key;
};

struct KeyUp
{
    KeyCode key;
};

using WindowEvent = std::variant<WindowResize, WindowClose>;
using InputEvent = std::variant<KeyDown, KeyUp>;
 

class EventQueue {
public:
    static EventQueue& Get() {
        static EventQueue instance;
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