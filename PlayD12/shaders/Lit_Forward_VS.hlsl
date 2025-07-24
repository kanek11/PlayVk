// Scene-level (b0)
cbuffer SceneCB : register(b0, space0)
{ 
    float4x4 ProjectionViewMatrix; 
};

// Object-level (b1)
cbuffer ObjectCB : register(b1, space0)
{
    float4x4 ModelMatrix; 
};
 

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL; 
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Offset : INSTANCE_OFFSET;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(ModelMatrix, float4(input.Position + input.Offset, 1.0f));
    output.position = mul(ProjectionViewMatrix, worldPos);
    output.texCoord = input.TexCoord;
    output.normal = normalize(mul((float3x3) ModelMatrix, input.Normal));
    output.color = input.Color;
    return output;
}

 