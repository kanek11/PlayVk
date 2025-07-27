 
#include "Common/Scene.hlsli"
#include "Common/Object.hlsli"

#include "PBR/GBuffer_Common.hlsli"


VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(gModelMatrix, float4(input.Position, 1.0f));
    output.Position = mul(gPVMatrix , worldPos);

    output.WorldPos = worldPos.xyz;
      
    output.WorldNormal = normalize(mul(gModelMatrix, float4(input.Normal, 0.0f)).xyz);
     
    output.WorldTangent = normalize(mul(gModelMatrix, float4(input.Tangent, 0.0f)).xyz);
    
    output.TangentSign = input.TangentSign;
    
    output.UV = input.TexCoord;

    return output;
}
