
cbuffer UISettingsCB : register(b0)
{
    bool useTexture;
    float padding[63];
}
 
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

 
Texture2D fontAtlas : register(t0);
SamplerState fontAtlasSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{  
    if(useTexture)
    {
        float4 texColor = fontAtlas.Sample(fontAtlasSampler, input.texCoord).rgba;
    //float texColor = fontAtlas.Sample(fontAtlasSampler, input.texCoord).r;
     
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