#pragma pack_matrix(row_major) 


// vertex attributes
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION; 
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
}; 


cbuffer MVP : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

cbuffer Light : register(b1)
{
    float3 lightDir; // World-space direction
    float pad1;
    float3 lightColor;
    float pad2;
};

// fallback defaults
static const float3 DefaultLightDir = normalize(float3(-0.5f, -1.0f, -0.3f));
static const float3 DefaultLightColor = float3(1.0f, 1.0f, 1.0f);


// vertex shader
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(float4(input.position, 1.0f), model);
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);

    output.normal = normalize(mul(float4(input.normal, 0.0f), model).xyz);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

// texture + sampler binding
Texture2D diffuseTex : register(t0);
SamplerState sampler0 : register(s0);

// pixel shader
float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightDir); // may be garbage if not bound
    float3 light = (length(L) < 0.01f) ? DefaultLightDir : L;
    float3 color = (length(lightColor) < 0.01f) ? DefaultLightColor : lightColor;

    float NdotL = saturate(dot(N, -light));
    float3 texColor = diffuseTex.Sample(sampler0, input.uv).rgb;
    if (texColor.r == 0 && texColor.g == 0 && texColor.b == 0)
        texColor = float3(1.0, 1.0, 1.0); // fallback to white

    float3 finalColor = texColor * input.color.rgb * color * NdotL;
    return float4(finalColor, input.color.a);
}
