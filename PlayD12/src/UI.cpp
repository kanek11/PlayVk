#include "PCH.h"
#include "UI.h"

#include "Application.h"
#include "InputSystem.h"

#include "Render/Renderer.h"

 
void UIButton::Render()
{
	auto uiRenderer = GameApplication::GetInstance()->GetRenderer()->uiRenderer;
	assert(uiRenderer != nullptr); 
	 
	Float4 color = { 1.0f, 1.0f, 1.0f, 0.5f };  
	if (state == UIState::Hovered)
	{
		OnHover.BlockingBroadCast();
		color = Color::Cyan; 
		color[3] = 0.5f;
	}

	uiRenderer->AddQuad(layout, color);
}

void UIButton::Tick(float delta)
{
	this->Render();
	//auto inputSystem = GameApplication::GetInstance()->GetInputSystem();

	//auto pointer = inputSystem->GetMousePointer();
	//

	//if (IsPointInRect(layout, pointer.x, pointer.y)) {
	//	//std::cout << "UI: pointer x:" << pointer.x << '\n';
	//	if (inputSystem->IsMouseButtonJustPressed(MouseButtonCode::ButtonLeft))
	//	{
	//		std::cout << "UI: left button clicked" << '\n';
	//		OnClick.BlockingBroadCast();
	//	}
	//}

}