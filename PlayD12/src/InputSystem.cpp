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

    keyStateRaw.fill(false);
    keyState.fill(ButtonFrame{});

    mouseButtonStateRaw.fill(false);
    mouseButtonState.fill(ButtonFrame{});

	m_axisState.fill(0.f); 

	m_gamepad = GamepadInput();
}



void InputSystem::OnUpdate()
{ 
    m_gamepad.Update(); //update gamepad state


    auto events = EventQueue::Get().Drain();
    //std::cout << "input sys: cached event number:" << events.size() << '\n';


    for (const auto& event : events) {
        std::visit(overloaded{
       [this](const FKeyDown& e) { OnKeyDown(e.key); },
       [this](const FKeyUp& e) { OnKeyUp(e.key); },
       [this](const FMouseButtonDown& e) { OnMouseButtonDown(e); },
       [this](const FMouseButtonUp& e) { OnMouseButtonUp(e); },
              [this](const FMouseMove& e) { OnMouseMove(e); },
            }, event);
    }

    for (size_t i = 0; i < MAX_KeyCode; ++i) {
        keyState[i].Update(keyStateRaw[i]);
    }

    for (size_t i = 0; i < MAX_MouseCode; ++i) {
        mouseButtonState[i].Update(mouseButtonStateRaw[i]);
    }


    //new:
	for (auto& [axis, binds] : DefaultAxisBindings){ 
		if (binds.empty()) continue;  
		m_axisState[(size_t)axis] = 0.f;

        for (const auto& bind : binds) {
            if (IsKeyDown(bind.key)) { 
                m_axisState[(size_t)axis] += bind.buttonScale;
            }
			if (m_gamepad.GetButtonState(bind.gamepadButton)) {
				m_axisState[(size_t)axis] += bind.buttonScale;
			} 
			if (bind.gamepadAxis != GamepadAxis::COUNT) {
				m_axisState[(size_t)axis] += m_gamepad.GetAxis(bind.gamepadAxis);
			}  
            //clamp the value to [-1, 1]
            m_axisState[(size_t)axis] = std::clamp(m_axisState[(size_t)axis], -1.f, 1.f);
        }
    
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
    return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Down
        || keyState[static_cast<size_t>(key)].state == ButtonFrameState::Pressed;
}

bool InputSystem::IsKeyUp(KeyCode key) const
{
    return keyState[static_cast<size_t>(key)].state == ButtonFrameState::Up ||
        keyState[static_cast<size_t>(key)].state == ButtonFrameState::Released;
}

void InputSystem::OnMouseButtonDown(FMouseButtonDown e)
{
    mouseButtonStateRaw[static_cast<size_t>(e.button)] = true;
    std::cout << "input system: button down" << '\n';

    //push a UI event to the queue
    UIMouseButtonDown uiEvent = UIMouseButtonDown{ e.button, e.x, e.y };
    UIEventQueue::Get().Push(uiEvent);

}

void InputSystem::OnMouseButtonUp(FMouseButtonUp e)
{
    mouseButtonStateRaw[static_cast<size_t>(e.button)] = false;
    std::cout << "input system: button up" << '\n';

    UIMouseButtonUp uiEvent = UIMouseButtonUp{ e.button, e.x, e.y };
    UIEventQueue::Get().Push(uiEvent);
}

void InputSystem::OnMouseMove(FMouseMove e)
{
    this->MouseX = e.x;
    this->MouseY = e.y;

    //std::cout << "input system: mouseX:" << e.x << '\n';
    UIMouseMove uiEvent = UIMouseMove{ e.x, e.y };
    UIEventQueue::Get().Push(uiEvent);
}

FPointer InputSystem::GetMousePointer()
{
    return FPointer(MouseX, MouseY);
}

bool InputSystem::IsMouseButtonJustPressed(MouseButtonCode button)
{
    //std::cout << static_cast<int>(mouseButtonState[static_cast<size_t>(button)].state) << '\n';
    return mouseButtonState[static_cast<size_t>(button)].state == ButtonFrameState::Pressed;
}

float InputSystem::GetAxis(EAxis axis) const
{
	//std::cout << "get axis value :" << m_axisState[static_cast<size_t>(axis)] << '\n';
	return m_axisState[static_cast<size_t>(axis)];
}



void WindowsInputSource::OnKeyUp(int key)
{
    if (WindowsKeyMap.contains(key)) {
        //inputSystem->OnKeyUp(WindowsKeyMap.at(key)); 

        InputEvent event = FKeyUp{ .key = WindowsKeyMap.at(key) };
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
        InputEvent event = FKeyDown{ .key = WindowsKeyMap.at(key) };
        EventQueue::Get().Push(event);
    }
    else
    {
        std::cerr << "key not found" << '\n';
    }
}