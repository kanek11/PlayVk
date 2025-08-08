#include "Common/Scene.hlsli"
 #include "Common/Object.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    //float4 color : COLOR;
    //float3 bary : TEXCOORD0; // NEW: Barycentric coordinates
}; 
 
float4 PSMain(PSInput input) : SV_TARGET
{
    return gColor;
    //return float4(1.0f, 1.0f, 1.0f, 1.0f); 
}