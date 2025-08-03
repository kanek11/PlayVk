
#include "Common/Samplers.hlsli"

cbuffer UISettingsCB : register(b0)
{
    int useTexture;
    float padding[63];
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
     
        if (texColor.r <= 0.0f)
        {
            texColor.a = 0.0f;

        }
        return float4(texColor);
    }
    else
    {
        return float4(input.color);
    }
    
    
    
    //return float4(texColor, texColor, texColor, 1.0f);
    //return float4(input.texCoord.x, input.texCoord.y, 0.0f, 1.0f);
    //
}