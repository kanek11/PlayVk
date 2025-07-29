 
#ifndef SCENE_HLSLI
#define SCENE_HLSLI

#include "SpaceBindings.hlsli"

// Scene-level (b0)
cbuffer SceneCB : register(b0)
{
    //view
    float4x4 gPVMatrix;
    float4x4 gInvProj;
    float4x4 gInvView;
    float3 gCameraPos; 
    //frame 
    float2 gResolution;
    float gTime;
    float gDeltaTime;
    //uint gFrameIndex;
    
    //env
    //sunlight
    float4x4 gLightPVMatrix;
    float3 gLightDir;
    float3 gLightColor;  
    float gLightIntensity;
    
    //float3 gAmbientColor; 
    //float gIBLIntensity; 
};



float3 UVToViewDir(float2 uv)
{
    // NDC [-1, 1]
    float2 ndc = uv * 2.0f - 1.0f;
    float4 clip = float4(ndc, 1, 1);
     
    float4 view = mul(gInvProj, clip);
    view /= view.w;
     
    float4 world = mul(gInvView, float4(view.xyz, 0));
    float3 dir = -normalize(world.xyz);
    
    return dir;
}


#endif