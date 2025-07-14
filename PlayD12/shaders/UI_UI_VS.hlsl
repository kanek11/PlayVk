cbuffer MVPConstantBuffer : register(b0)
{
    float4x4 modelMatrix;
    float4x4 pvMatrix;
    float4 padding[6];
};

struct VSInput
{
    float2 position : POSITION; 
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;  
};

struct PSInput
{
    float4 position : SV_POSITION; 
    float2 texCoord : TEXCOORD; 
    float4 color : COLOR;
};
  

PSInput VSMain(VSInput input)
{
    PSInput result;

    float4 worldPosition = mul(modelMatrix, float4(input.position.xy, 0.0f, 1.0f));
    result.position = worldPosition;
    result.color = input.color; 
    result.texCoord = input.texCoord;

    return result;
}


