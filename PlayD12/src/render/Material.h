#pragma once
#include "PCH.h"



//a dummy type
using TextureHandle = std::string; 

struct MaterialDesc {
    std::string name;
    std::string shaderTag;
    std::unordered_map<std::string, TextureHandle> textures;
    //std::unordered_map<std::string, float> scalarParams;
    //std::unordered_map<std::string, bool> keywords; // like USE_NORMAL_MAP

	//todo: other blend mode;
    bool enableAlphaBlend = false;
    bool doubleSided = false;
    bool depthWrite = true;
};



// a simple solution of material registry;
namespace Materials {
     
    inline MaterialDesc shadowMaterialDesc = {
        .name = "Shadow",
        .shaderTag = "Lit",

        .textures = {
            {"baseMap", "default_shadow_texture"}, // Placeholder texture handle
        },

        .enableAlphaBlend = false,
        .doubleSided = false,
        .depthWrite = true 
    };
     
}
