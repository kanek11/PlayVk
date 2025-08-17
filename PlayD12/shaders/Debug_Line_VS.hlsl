#include "Common/Scene.hlsli" 

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(gPVMatrix, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}
