 
#include "Common/Scene.hlsli" 
#include "Common/Object.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float TangentSign : TANGENT_SIGN;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR;
    float3 Offset : INSTANCE_OFFSET;
};


struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(gModelMatrix, float4(input.Position, 1.0f));
    output.position = mul(gLightPVMatrix, worldPos);
    return output;
}