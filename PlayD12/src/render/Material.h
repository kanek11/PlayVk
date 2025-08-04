#pragma once
#include "PCH.h"

#include "Math/MMath.h"
#include "Base.h"

#include "Texture.h" 

//a dummy type 
 

struct MaterialDesc {
    std::string name;
    std::string shaderTag;
    //std::unordered_map<std::string, TextureHandle> textures;
    //std::unordered_map<std::string, float> scalarParams;
    //std::unordered_map<std::string, bool> keywords; // like USE_NORMAL_MAP

    //todo: other blend mode;
    bool enableAlphaBlend = false;
    bool doubleSided = false;
    bool depthWrite = true;
}; 

 
// a simple solution of material registry;
namespace Materials {

    struct alignas(256) PBRMaterialCB
    {
        Float3 baseColor = Float3(1.0f, 1.0f, 1.0f);
        uint32_t useBaseColorMap{ 1 };

        float ao{ 0.0f };
        uint32_t useAOMap{ 0 };

        uint32_t useNormalMap{ 0 };

        float roughness{ 0.0f };
        uint32_t useRoughnessMap{ 0 };

        float metallic{ 0.1f };
        uint32_t useMetallicMap{ 0 };

        float emissiveStrength{ 0 };
        alignas(16) Float3 emissiveColor = Float3(0.0f, 0.0f, 0.0f);
        uint32_t useEmissiveMap{ 0 };
    };

}

struct UMaterial {

    Materials::PBRMaterialCB materialCB;

    // texture name is directly the semantics in shader;
    std::unordered_map<std::string, std::string > textures;
    //SharedPtr<FD3D12Texture> baseMap;

    UMaterial();
    
    //void DeriveParameters();
};


