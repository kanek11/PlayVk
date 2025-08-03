
struct VSInput
{
    float2 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
}; 

VSOutput VSMain(VSInput input)
{
    VSOutput result;
    //float4 worldPosition = mul(modelMatrix, float4(input.position.xy, 0.0f, 1.0f));
    float4 worldPosition = float4(input.position.xy, 0.0f, 1.0f);
    result.position = worldPosition;
    result.color = input.color;
    result.texCoord = input.texCoord;

    return result;
}

