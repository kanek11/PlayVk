#pragma once
#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"
#include "Loader.h"

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
    std::optional<Loader::D3D12ImageData> imageData;
    SharedPtr<FD3D12Texture> texture;

    [[nodiscard]]
    bool LoadTexture(const std::string& filePath);
    //void CreateResource();

    void LoadGridAtlas(float cellWidth, float cellHeight,
        size_t columns, size_t rows);
};