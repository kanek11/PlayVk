#include "PCH.h"
#include "UI.h"

#include "Application.h"
#include "InputSystem.h"

#include "Render/Renderer.h"
#include "Render/Text.h"


void UIButton::RenderBack()
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

void UIButton::RenderText()
{
	auto uiRenderer = GameApplication::GetInstance()->GetRenderer()->uiRenderer;
	assert(uiRenderer != nullptr);

	SharedPtr<FontAtlas> font = uiRenderer->font;
	if (!font) {
		std::cout << "UIRenderer font is not set!" << std::endl;
		return;
	}

	Float2 size = { static_cast<float>(layout.h), static_cast<float>(layout.h) };
	float advance = static_cast<float>(layout.h);

	Float2 cursor = { static_cast<float>(layout.x), static_cast<float>(layout.y) };
	for (char c : text) {
		if (!font->glyphs.contains(c)) continue;

		const Glyph& g = font->glyphs[c];

		FRect glyphRect = {
			static_cast<int>(cursor.x() + g.bearing.x()),
			static_cast<int>(cursor.y() + g.bearing.y()),
			static_cast<int>(size.x()),
			static_cast<int>(size.y())
			//static_cast<int>(g.size.x()),
			//static_cast<int>(g.size.y())
		};

		uiRenderer->AddQuad(glyphRect, g.uvTopLeft, g.uvBottomRight);

		//cursor = { cursor.x() + g.advance, cursor.y() };
		cursor = { cursor.x() + advance, cursor.y() };
	}
}
 

void UIButton::Tick(float delta)
{
	//this->RenderBack();
	this->RenderText();

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