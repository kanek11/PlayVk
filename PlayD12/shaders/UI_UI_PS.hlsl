
#include "Common/Samplers.hlsli"

cbuffer UISettingsCB : register(b0)
{
    int useTexture;
    float opacity; 
}
 
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

 
Texture2D baseColorMap : register(t0); 

float4 PSMain(VSOutput input) : SV_TARGET
{  
    if(useTexture > 0)
    {
        float4 texColor = baseColorMap.Sample(pointClampSampler, input.texCoord).rgba;
 
        float3 color = texColor.xyz * input.color.xyz;
        float alpha = texColor.a * input.color.a * opacity;
        return float4(color, alpha);
    }
    else
    {
        return float4(input.color.xyz, input.color.a * opacity);
    } 
}