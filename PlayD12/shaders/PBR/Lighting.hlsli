#ifndef PBR_Lighting_HLSLI
#define PBR_Lighting_HLSLI

struct DirectionalLight
{
    float3 direction;
    float3 color;
    float intensity;
};

#endif