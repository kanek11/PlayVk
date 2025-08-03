#ifndef COMMON_OBJECT_HLSLI
#define COMMON_OBJECT_HLSLI

#include "SpaceBindings.hlsli" 

// Object-level (b1)
cbuffer ObjectCB : register(b1)
{
    float4x4 gModelMatrix;
    float4x4 gNormalMatrix; // transpose(inverse(gModelMatrix))
    float3 gCenterPos; //for probe;
};
  
#endif