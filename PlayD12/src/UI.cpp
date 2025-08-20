#include "PCH.h"
#include "UI.h"

#include "Application.h"
#include "InputSystem.h"

#include "Render/Renderer.h"
#include "Render/Text.h"

std::atomic<uint32_t> UIElement::GIdGenerator{ 0 };

UIElement::UIElement()
{
	baseColor[3] = 0.5f;  

	//set id:
	id = ++GIdGenerator;
}


UIButton::UIButton() : UIElement()
{
	OnHoverEnter.Add([this]() {
		baseColor = Color::Cyan;
		baseColor[3] = 0.5f;
		});

	OnHoverExit.Add([this]() {
		baseColor = { 1.0f, 1.0f, 1.0f, 0.5f };
		});
}

void UIButton::RenderBack()
{
	UIElement::RenderBack();

	auto renderer = GameApplication::GetInstance()->GetRenderer();
	assert(renderer != nullptr);
	assert(layout.has_value());

	renderer->AddQuad(layout.value(), baseColor);
}

void UIButton::RenderText()
{
	auto renderer = GameApplication::GetInstance()->GetRenderer();
	assert(renderer != nullptr);

	assert(layout.has_value());
	auto layout = this->layout.value();

	SharedPtr<FontAtlas> font = renderer->GetFontAtlas();
	if (!font) {
		std::cout << "UIRenderer font is not set!" << std::endl;
		return;
	}

	Float2 size = { static_cast<float>(layout.h), static_cast<float>(layout.h) };
	float advance = static_cast<float>(layout.h);

	Float2 cursor = { static_cast<float>(layout.x), static_cast<float>(layout.y) };

	//clip the text only up to ,say only 10:
	std::string text_ = this->text.substr(0, 20); // Limit to first 10 characters

	for (char c : text_) {
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

		renderer->AddQuad(glyphRect, g.uvTopLeft, g.uvBottomRight);

		//cursor = { cursor.x() + g.advance, cursor.y() };
		cursor = { cursor.x() + advance, cursor.y() };
	}
}


void UIButton::Tick(float delta)
{
	UIElement::Tick(delta);

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

UICanvasPanel::UICanvasPanel() : UIElement()
{
}

void UICanvasPanel::Tick(float delta)
{
	UIElement::Tick(delta);

	//auto renderer = GameApplication::GetInstance()->GetRenderer();
	//assert(renderer != nullptr);
	//assert(layout.has_value());

	//renderer->AddQuad(layout.value(), baseColor);
}