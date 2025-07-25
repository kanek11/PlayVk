// Scene-level (b0)
cbuffer SceneCB : register(b0, space0)
{
    float4x4 PVMatrix;
    float4x4 LightPVMatrix;
    float3 LightDirection;
    float3 LightColor;
    
    float padding[26];
};

// Object-level (b1)
cbuffer ObjectCB : register(b1, space0)
{
    float4x4 ModelMatrix;
};
 

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(ModelMatrix, float4(input.position, 1.0f));
    output.position = mul(LightPVMatrix, worldPos);
    return output;
}