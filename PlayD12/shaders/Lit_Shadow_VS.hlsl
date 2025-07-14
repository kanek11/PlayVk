cbuffer MVPConstantBuffer : register(b0)
{
    float4x4 modelMatrix;
    float4x4 lightProjViewMatrix;
    float4 padding[6];
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
    float4 worldPos = mul(modelMatrix, float4(input.position, 1.0f));
    output.position = mul(lightProjViewMatrix, worldPos);
    return output;
}