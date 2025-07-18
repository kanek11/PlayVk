#pragma once
#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"

struct Glyph {
    Float2 uvTopLeft;
    Float2 uvBottomRight;
    Float2 size; 
    Float2 bearing;
    float advance;
};

class FontAtlas {
public:
    std::unordered_map<char, Glyph> glyphs;
    SharedPtr<FD3D12Texture> texture;

    void LoadTexture(const std::string& filePath);
    void LoadGridAtlas(int cellWidth, int cellHeight,
        int columns, int rows);
};
