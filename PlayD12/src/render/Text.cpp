#include "PCH.h"
#include "Text.h"

void FontAtlas::LoadTexture(const std::string& filePath)
{
}

void FontAtlas::LoadGridAtlas(
    int cellWidth, int cellHeight,
    int columns, int rows)
{

    auto texDesc = texture->GetDesc();
	auto texWidth = texDesc.width;
	auto texHeight = texDesc.height;

    // Fill ASCII [32–127)
    for (int i = 0; i < 96; ++i) {
        char ch = static_cast<char>(32 + i);
        int col = i % columns;
        int row = i / columns;

        float u0 = (col * cellWidth) / static_cast<float>(texWidth);
        float v0 = (row * cellHeight) / static_cast<float>(texHeight);
        float u1 = ((col + 1) * cellWidth) / static_cast<float>(texWidth);
        float v1 = ((row + 1) * cellHeight) / static_cast<float>(texHeight);


        //hard code for now;
        glyphs[ch] = Glyph{
            .uvTopLeft = { u0, v0 },
            .uvBottomRight = { u1, v1 },
            .size = { static_cast<float>(cellWidth), static_cast<float>(cellHeight) },
            .bearing = { 0.0f, 0.0f },
            .advance = static_cast<float>(cellWidth),
        };
    }

}
