// Example: Vulkan HLSL version

// Input vertex attributes
struct VSInput
{
    float3 inPos : POSITION;
    float2 UV0 : TEXCOORD0;
    float3 inNormal : NORMAL; // You can map extra semantics if needed
    float3 inTangent : TANGENT;
};

// Output to rasterizer
struct VSOutput
{
    float4 gl_Position : SV_POSITION; // Vulkan expects this name for SPIR-V translation
    float2 UV0 : TEXCOORD0;
};

// Uniform buffer (set = 0, binding = 0)
cbuffer CameraUBO : register(b0, space0)
{
    float4x4 view;
    float4x4 proj;
}

// Push constant (Vulkan HLSL syntax)
struct PushConstants
{
    float4x4 model;
};

// instantiate as global variable, attribute push-constant
[[vk::push_constant]]
PushConstants PC;

// Vertex shader entry
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 _Position = float4(input.inPos, 1.0);
    output.gl_Position = mul(proj, mul(view, mul(PC.model, _Position)));
    output.UV0 = input.UV0;

    return output;
}


// Declare combined sampler
// Vulkan HLSL: descriptor set & binding by [[vk::binding(X, Y)]]
// Vulkan binding: set = 1, binding = 0 
//--------------------------------------------- 
//[[vk::binding(0, 1)]]
[[vk::combinedImageSampler]]   
Texture2D Sampler0 : register(t0, space1);

// marked as combined image sampler
//[[vk::binding(0, 1)]] 
[[vk::combinedImageSampler]]   
SamplerState SamplerState0 : register(s0, space1); 
 
// Output
float4 PSMain(VSOutput input) : SV_TARGET0
{  
    // Sample texture
    float3 _color = Sampler0.Sample(SamplerState0, input.UV0).rgb;

    return float4(_color, 1.0);
}
