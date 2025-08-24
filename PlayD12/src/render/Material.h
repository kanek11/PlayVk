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
    bool enableWireFrame = false;
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

        float emissiveStrength{ 1.0f };
        alignas(16) Float3 emissiveColor = Float3(0.0f, 0.0f, 0.0f);
        uint32_t useEmissiveMap{ 0 }; 

    };

}

struct UMaterial {

    Materials::PBRMaterialCB materialCB;

    // texture name is directly the semantics in shader;
    std::unordered_map<std::string, std::string > textures;
    //SharedPtr<FD3D12Texture> baseMap; 

    //new:
    bool transparent{ false };
    Float4 transparentColor{ 1.0f, 0.0f, 0.0f, 1.0f };

    UMaterial();
    //void DeriveParameters();
};



namespace Materials {


    inline SharedPtr<UMaterial> GetIron() {

        auto Mat = CreateShared<UMaterial>();

        Mat->materialCB.useBaseColorMap = false;
        Mat->materialCB.baseColor = Color::Iron;

        Mat->materialCB.useNormalMap = false;
        Mat->materialCB.useMetallicMap = false;
        Mat->materialCB.metallic = 1.0f;

        Mat->materialCB.useRoughnessMap = false;
        Mat->materialCB.roughness = 0.0f;

        Mat->materialCB.useAOMap = false;

        return Mat;

    }


    inline SharedPtr<UMaterial> GetRustyIron() {

        auto Mat = CreateShared<UMaterial>();

        Mat->textures["baseColorMap"] = "rusty_metal_04_diff_1k.png";
        Mat->textures["normalMap"] = "rusty_metal_04_nor_dx_1k.png";
        Mat->textures["metallicMap"] = "rusty_metal_04_metal_1k.png";
        Mat->textures["RoughnessMap"] = "rusty_metal_04_rough_1k.png";
        Mat->textures["AOMap"] = "rusty_metal_04_ao_1k.png";

        Mat->materialCB.useBaseColorMap = true;
        Mat->materialCB.useNormalMap = true;
        Mat->materialCB.useMetallicMap = true;
        Mat->materialCB.useRoughnessMap = true;
        Mat->materialCB.useAOMap = true;

        return Mat;
    }


    inline SharedPtr<UMaterial> GetSnowSurface() {

        auto Mat = CreateShared<UMaterial>();

        Mat->textures["baseColorMap"] = "Snow001_1K-JPG_Color.jpg";
        Mat->textures["normalMap"] = "Snow001_1K-JPG_NormalDX.jpg";
        Mat->textures["RoughnessMap"] = "Snow001_1K-JPG_Roughness.jpg";
        //Mat->textures["AOMap"] = "snow_02_ao_1k.png";

        Mat->materialCB.useBaseColorMap = true;
        Mat->materialCB.useNormalMap = true;
        Mat->materialCB.useMetallicMap = false;
        Mat->materialCB.metallic = 0.0f;
        Mat->materialCB.useRoughnessMap = true;
        Mat->materialCB.useAOMap = false;

        return Mat;
    }

    inline SharedPtr<UMaterial> GetIceSurface() {

        auto Mat = CreateShared<UMaterial>();

        Mat->textures["baseColorMap"] = "Ice003_1K-JPG_Color.jpg";
        Mat->textures["normalMap"] = "Ice003_1K-JPG_NormalDX.jpg";
        Mat->textures["RoughnessMap"] = "Ice003_1K-JPG_Roughness.jpg";
        //Mat->textures["AOMap"] = "snow_02_ao_1k.png";


        Mat->materialCB.useBaseColorMap = true;
        Mat->materialCB.useNormalMap = true;
        Mat->materialCB.useMetallicMap = false;
        Mat->materialCB.metallic = 0.1f;
        Mat->materialCB.useRoughnessMap = true;
        Mat->materialCB.useAOMap = false;

        return Mat;
    }


    inline SharedPtr<UMaterial> GetPlayerMat() {

        auto Mat = CreateShared<UMaterial>();

        //Mat->textures["baseColorMap"] = "PlayerFace";
        //Mat->textures["normalMap"] = "rusty_metal_04_nor_dx_1k.png";
        //Mat->textures["metallicMap"] = "rusty_metal_04_metal_1k.png";
        //Mat->textures["RoughnessMap"] = "rusty_metal_04_rough_1k.png";
        //Mat->textures["AOMap"] = "rusty_metal_04_ao_1k.png";
        Mat->textures["emissionMap"] = "PlayerFace";

        Mat->materialCB.useBaseColorMap = false;
        Mat->materialCB.baseColor = Color::Blue.xyz();

        Mat->materialCB.useNormalMap = false;
        Mat->materialCB.useMetallicMap = false;
        Mat->materialCB.metallic = 0.1f;

        Mat->materialCB.useRoughnessMap = false;
        Mat->materialCB.roughness = 0.2f;

        Mat->materialCB.useAOMap = false;

        Mat->materialCB.useEmissiveMap = true;

        return Mat;

    }


    inline SharedPtr<UMaterial> AttachPlayer(SharedPtr<UMaterial> Mat) {
           
        Mat->textures["emissionMap"] = "PlayerFace"; 

        Mat->materialCB.useEmissiveMap = true; 

        return Mat;
    }


    inline SharedPtr<UMaterial> GetTransparentMesh() {

        auto Mat = CreateShared<UMaterial>();

        Mat->transparent = true;

        return Mat;
    }

}