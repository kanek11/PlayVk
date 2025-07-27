 
#ifndef SCENE_HLSLI
#define SCENE_HLSLI

#include "SpaceBindings.hlsli"

// Scene-level (b0)
cbuffer SceneCB : register(b0)
{
    //view
    float4x4 gPVMatrix;
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

#endif