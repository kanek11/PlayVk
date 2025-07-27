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
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR; 
    float4 lightSpacePos : TEXCOORD1;
    
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(gModelMatrix, float4(input.Position + input.Offset, 1.0f));
    output.position = mul(gPVMatrix, worldPos);
    output.texCoord = input.TexCoord;
    output.normal = normalize(mul((float3x3) gModelMatrix, input.Normal));
    output.color = input.Color;
     
    output.lightSpacePos = mul(gLightPVMatrix, worldPos);
    
    return output;
}

 