#ifndef PBR_MATERIAL_HLSLI
#define PBR_MATERIAL_HLSLI

#include "../Common/SpaceBindings.hlsli"

struct PBRParameters
{
    float3 baseColor;
    uint useBaseColorMap;
     
    float ao;
    uint useAOMap;
    
    uint useNormalMap;
    
    float roughness;
    uint useRoughnessMap;
    
    float metallic;
    uint useMetallicMap;
    
    float emissiveStrength;
    float3 emissiveColor;
    uint useEmissiveMap;
    
    //normal map tweaking
    //float2 uvScale;
    //float2 uvOffset; 

    // Feature flags (bitmask): CLEARCOAT, ANISO, TRANSMISSION...
    //uint features; 
    
    // Future use 
    //float opacity; 
};

 
cbuffer MaterialCB : register(b2)
{
    PBRParameters gMaterial;
};
 
// Material parameters
Texture2D baseColorMap : register(t0, space1);
Texture2D normalMap : register(t1, space1);
Texture2D metallicMap : register(t2, space1);
Texture2D RoughnessMap : register(t3, space1);
Texture2D AOMap : register(t4, space1);

SamplerState linearWrapSampler : register(s0);

float3 GetBaseColor(float2 uv)
{
    return gMaterial.useBaseColorMap ? baseColorMap.Sample(linearWrapSampler, uv).rgb : gMaterial.baseColor;
}

float GetMetallic(float2 uv)
{
    return gMaterial.useMetallicMap ? metallicMap.Sample(linearWrapSampler, uv).r : gMaterial.metallic;
}

float GetRoughness(float2 uv)
{
    return gMaterial.useRoughnessMap ? RoughnessMap.Sample(linearWrapSampler, uv).r : gMaterial.roughness;
}

float GetAO(float2 uv)
{
    return gMaterial.useAOMap ? AOMap.Sample(linearWrapSampler, uv).r : gMaterial.ao;
}

float3 GetEmissive(float2 uv)
{
    if (gMaterial.useEmissiveMap)
    {
        return baseColorMap.Sample(linearWrapSampler, uv).rgb * gMaterial.emissiveStrength;
    }
    return gMaterial.emissiveColor * gMaterial.emissiveStrength;
}


#endif