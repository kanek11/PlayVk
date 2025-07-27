#ifndef OBJECT_HLSLI
#define OBJECT_HLSLI

#include "SpaceBindings.hlsli" 

// Object-level (b1)
cbuffer ObjectCB : register(b1)
{
    float4x4 gModelMatrix;
    float4x4 gNormalMatrix; // transpose(inverse(gModelMatrix))
};
 

#endif