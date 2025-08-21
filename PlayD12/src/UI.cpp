#include "PCH.h"
#include "UI.h"

#include "Application.h"
#include "InputSystem.h"

#include "Render/Renderer.h"
#include "Render/Text.h"

std::atomic<uint32_t> UIElement::GIdGenerator{ 0 };

UIElement::UIElement()
{ 
	//set id:
	id = ++GIdGenerator;
}


UITextBlock::UITextBlock() : UIElement()
{
}

void UITextBlock::RenderBack()
{
	UIElement::RenderBack();

	auto renderer = GameApplication::GetInstance()->GetRenderer();
	assert(renderer != nullptr);
	assert(layout.has_value());

	renderer->AddQuad(layout.value(), { baseColor.x(), baseColor.y(), baseColor.z(), opacity });
}

void UITextBlock::RenderText()
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
	std::string text_ = this->text.substr(0, 20); 

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

void UITextBlock::DeriveAutoText()
{ 
	if (!style.has_value() || style.value().policy != UI::SizePolicy::AutoText)
	{
		return;
	}

	assert(layout.has_value());
	auto layout = this->layout.value();

	float lineHeightPx = layout.h;
	float w = (float)text.size() * lineHeightPx; 

	this->layout.value().w = static_cast<int>(w);
}


void UITextBlock::Tick(float delta)
{
	UIElement::Tick(delta); 

	this->DeriveAutoText();
	this->RenderBack();
	this->RenderText(); 
}

UICanvasPanel::UICanvasPanel() : UIElement()
{
}

void UICanvasPanel::Tick(float delta)
{
	UIElement::Tick(delta); 
}