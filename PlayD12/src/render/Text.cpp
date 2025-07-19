#include "PCH.h"
#include "Text.h"

#include "Loader.h"
  
void FontAtlas::LoadTexture(const std::string& filePath)
{
    if (auto texData = Loader::LoadTextureDX(filePath); texData.has_value())
    {
        this->imageData = texData;
    }
    else
    {

    }


}

void FontAtlas::LoadGridAtlas(
    float cellWidth, float cellHeight,
    size_t columns, size_t rows)
{

    auto texDesc = texture->GetDesc();
    auto texWidth = texDesc.width;
    auto texHeight = texDesc.height;

    // Fill ASCII [32 to 127)
    for (size_t i = 0; i < 96; ++i) {
        char ch = static_cast<char>(32 + i);
        size_t col = i % columns;
        size_t row = i / columns;

        float u0 = (col * cellWidth) / static_cast<float>(texWidth);
        float v0 = (row * cellHeight) / static_cast<float>(texHeight);
        float u1 = ((col + 1) * cellWidth) / static_cast<float>(texWidth);
        float v1 = ((row + 1) * cellHeight) / static_cast<float>(texHeight);

         
        glyphs[ch] = Glyph{
            .uvTopLeft = { u0, v0 },
            .uvBottomRight = { u1, v1 },
            .size = { static_cast<float>(cellWidth), static_cast<float>(cellHeight) },
            .bearing = { 0.0f, 0.0f },
            .advance = static_cast<float>(cellWidth),
        };
    }

}