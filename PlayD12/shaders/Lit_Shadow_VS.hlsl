 
#include "Common/Scene.hlsli" 
#include "Common/Object.hlsli"

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(gModelMatrix, float4(input.position, 1.0f));
    output.position = mul(gLightPVMatrix, worldPos);
    return output;
}