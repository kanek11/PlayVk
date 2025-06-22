struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer Camera : register(b0)
{
    float4x4 vpMatrix;
};

Texture2D diffuseMap : register(t0);
SamplerState linearClamp : register(s0);

VSOutput VSMain(VSInput input)
{
    VSOutput o;
    o.posH = mul(vpMatrix, float4(input.pos, 1.0));
    o.uv = input.uv;
    return o;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return diffuseMap.Sample(linearClamp, input.uv);
}
