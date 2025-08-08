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

struct PSInput
{
    float4 position : SV_POSITION;  
    //float4 color : COLOR; 
    //float3 bary : TEXCOORD0; // NEW: Barycentric coordinates
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(gModelMatrix, float4(input.Position, 1.0f));
    output.position = mul(gPVMatrix, worldPos); 
    //output.color = input.Color; 
    
    return output;
}

 