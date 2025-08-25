#include "PCH.h"
#include "UI.h"

#include "Application.h"
#include "InputSystem.h"

#include "Render/Renderer.h"
#include "Render/Text.h"

#include "Math/AnimUtils.h"


using namespace UI;


std::atomic<uint32_t> UIElement::GIdGenerator{ 0 };

UIElement::UIElement()
{
	//set id:
	id = ++GIdGenerator;
}

void UIElement::RenderBack()
{
	auto renderer = GameApplication::GetInstance()->GetRenderer();
	assert(renderer != nullptr);
	assert(layout.has_value());

	FQuadDesc desc = {
	.rect = layout.value(),
	.uvTL = Float2(0.0f, 0.0f),
	.uvBR = Float2(1.0f, 1.0f),
	.color = baseColor,
	.alpha = backAlpha,
	.opacity = opacity,
	.useTexture = backTex.has_value(),
	.texName = backTex.value_or("Checkerboard"),
	};

	renderer->AddQuad(desc);
}

void UIElement::InvokeClick(bool vibrate)
{
	OnClick.BlockingBroadCast();

	//if(vibrate)
	//Anim::VibrateFor(0.4f, 0.4f, 0.2f);
}

UITextBlock::UITextBlock() : UIElement()
{
}

void UITextBlock::RenderBack()
{
	UIElement::RenderBack();


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

	float offset = static_cast<float>(layout.h);

	Float2 size = { static_cast<float>(layout.h) , static_cast<float>(layout.h) };
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

		FQuadDesc desc = {
.rect = glyphRect,
.uvTL = g.uvTopLeft,
.uvBR = g.uvBottomRight,
.color = baseColor,
.alpha = {1.0f,1.0f,1.0f,1.0f},
.opacity = opacity,
.useTexture = true,
.texName = "ASCII_16x6.png"
		};

		renderer->AddQuad(desc);

		//cursor = { cursor.x() + g.advance, cursor.y() };
		float advanceX = std::clamp(cursor.x() + advance, (float)layout.x, (float)layout.x + layout.w);
		cursor = { cursor.x() + advance, cursor.y() };
	}
}

void UITextBlock::ApplyAutoText()
{
	if (style.policy != UI::SizePolicy::AutoText)
	{
		return;
	}

	assert(layout.has_value());
	auto layoutV = this->layout.value();

	float lineHeightPx = (float)layoutV.h;
	float targetW = (float)text.size() * lineHeightPx ;

	//if (targetW > layoutV.w)  
	this->layout.value().w = static_cast<int>(targetW);


	//if (text.size() > 0) {
	//	this->style.width = 
	//		style.height.unit == Unit::Percent ? 
	//		UISize{ Unit::Percent, style.height.value * text.size() }:
	//		UISize{ Unit::Pixel, style.height.value * text.size() }
	//	;
	//}

}


void UITextBlock::Tick(float delta)
{
	UIElement::Tick(delta);

	this->RenderBack();
	this->RenderText();
}

void UITextBlock::ResolvePixelRect()
{
	UIElement::ResolvePixelRect();

	this->ApplyAutoText();


	if (style.alignment == Alignment::Center) {
		if (this->layout.has_value() && m_parent && m_parent->GetLayout().has_value())
			CenterRectX(this->layout.value(), m_parent->GetLayout().value());
	}
}

UICanvasPanel::UICanvasPanel() : UIElement()
{
}

void UICanvasPanel::Tick(float delta)
{
	UIElement::Tick(delta);

	this->RenderBack();
}