// DebugDraw.hlsl
cbuffer MVPConstantBuffer : register(b0)
{
    float4x4 modelMatrix; 
    float4x4 vpMatrix;  
    float4 padding[6];  
};
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(vpMatrix, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
