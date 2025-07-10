#include "PCH.h"
#include "InputSystem.h"


//windows scancode is basically ASCII:
static const std::unordered_map<int, KeyCode> WindowsKeyMap = {
    // Letters
    { 'A', KeyCode::A }, { 'B', KeyCode::B }, { 'C', KeyCode::C }, { 'D', KeyCode::D },
    { 'E', KeyCode::E }, { 'F', KeyCode::F }, { 'G', KeyCode::G }, { 'H', KeyCode::H },
    { 'I', KeyCode::I }, { 'J', KeyCode::J }, { 'K', KeyCode::K }, { 'L', KeyCode::L },
    { 'M', KeyCode::M }, { 'N', KeyCode::N }, { 'O', KeyCode::O }, { 'P', KeyCode::P },
    { 'Q', KeyCode::Q }, { 'R', KeyCode::R }, { 'S', KeyCode::S }, { 'T', KeyCode::T },
    { 'U', KeyCode::U }, { 'V', KeyCode::V }, { 'W', KeyCode::W }, { 'X', KeyCode::X },
    { 'Y', KeyCode::Y }, { 'Z', KeyCode::Z },

    // Numbers
    { '0', KeyCode::Num0 }, { '1', KeyCode::Num1 }, { '2', KeyCode::Num2 },
    { '3', KeyCode::Num3 }, { '4', KeyCode::Num4 }, { '5', KeyCode::Num5 },
    { '6', KeyCode::Num6 }, { '7', KeyCode::Num7 }, { '8', KeyCode::Num8 },
    { '9', KeyCode::Num9 },

    // Special keys
    { VK_SPACE,     KeyCode::Space },
    { VK_RETURN,    KeyCode::Enter },
    { VK_ESCAPE,    KeyCode::Escape },
    { VK_TAB,       KeyCode::Tab },
    { VK_BACK,      KeyCode::Backspace },

    // Arrows
    { VK_LEFT,  KeyCode::Left },
    { VK_RIGHT, KeyCode::Right },
    { VK_UP,    KeyCode::Up },
    { VK_DOWN,  KeyCode::Down },

    // Add more mappings if needed
};

//initialize known code;
InputSystem::InputSystem() { 
     
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void InputSystem::OnUpdate()
{
    auto events = EventQueue::Get().Drain();
     

    for (const auto& event : events) {
        std::visit( overloaded {
       [this](const KeyDown& e) { OnKeyDown(e.key); },
       [this](const KeyUp& e) { OnKeyUp(e.key); }, 
            }, event);
    }
     
	for (size_t i = 0; i < MaxKeyCode; ++i) {
		keyState[i].Update(keyStateRaw[i]);
	}
}

void InputSystem::OnKeyDown(KeyCode key)
{ 
    keyStateRaw[static_cast<size_t>(key)] = true;
}

void InputSystem::OnKeyUp(KeyCode key)
{
	keyStateRaw[static_cast<size_t>(key)] = false;
     
}

bool InputSystem::IsKeyJustPressed(KeyCode key) const
{  
	return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Pressed;
}

bool InputSystem::IsKeyJustReleased(KeyCode key) const
{
	return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Released;
}

bool InputSystem::IsKeyDown(KeyCode key) const
{
	return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Down ||
		keyState[static_cast<size_t>(key)].state == ButtonFrameState::Pressed;
}

bool InputSystem::IsKeyUp(KeyCode key) const
{
	return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Up ||
		keyState[static_cast<size_t>(key)].state == ButtonFrameState::Released;
}



void WindowsInputSource::OnKeyUp(int key)
{
    if (WindowsKeyMap.contains(key)) {
        //inputSystem->OnKeyUp(WindowsKeyMap.at(key)); 
 
        InputEvent event = KeyUp{ .key = WindowsKeyMap.at(key) };
        EventQueue::Get().Push(event);
    }
    else
    {
        std::cerr << "key not found" << '\n';
    }
}

void WindowsInputSource::OnKeyDown(int key)
{
    if (WindowsKeyMap.contains(key)) {
        //inputSystem->OnKeyDown(WindowsKeyMap.at(key)); 
        InputEvent event = KeyDown{ .key = WindowsKeyMap.at(key) };
        EventQueue::Get().Push(event);
    }
    else
    {
        std::cerr << "key not found" << '\n';
    }
}
