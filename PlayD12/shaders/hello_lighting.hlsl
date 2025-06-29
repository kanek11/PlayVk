


cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset; // 16 bytes
    float4x4 vpMatrix; // 64 bytes
    float4 padding[11]; // 176 bytes
};
// Total: 256 bytes

struct VSInput
{
    float3 position : POSITION; // Per-vertex
    float3 normal : NORMAL;
    float3 tangent : TANGENT; // Not used in this shader
    float4 color : COLOR; // Per-vertex
    float2 texCoord : TEXCOORD; // Per-vertex
    float3 instanceOffset : INSTANCE_OFFSET; // Per-instance
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

Texture2D baseMap : register(t0);
SamplerState baseMapSampler : register(s0);

static const float3 lightDirection = normalize(float3(0.577f, 0.277f, 0.377f)); // Example light dir
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f); // White light

PSInput VSMain(VSInput input)
{
    PSInput result;

    float4 worldPosition = float4(input.position.xyz + input.instanceOffset, 1.0f);
    result.position = mul(vpMatrix, worldPosition);
    result.color = input.color;

    //result.normal = normalize(mul((float3x3) vpMatrix, input.normal.xyz));
    //we don't have model rotation, so we can just use the normal as is
    result.normal = input.normal.xyz;
    result.texCoord = input.texCoord;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Sample texture color
    float4 texColor = baseMap.Sample(baseMapSampler, input.texCoord);

    // Basic Lambert lighting
    float NdotL = max(dot(input.normal, lightDirection), 0.0f);
    float ambient = 0.3f;

    float3 diffuse = lightColor * NdotL;
    float3 lighting = ambient + diffuse;

    // Multiply texture color by vertex color and lighting
    //float3 finalColor = input.color.rgb * lighting;
    //float3 finalColor = lighting * input.color.rgb;
    float3 finalColor = texColor.xyz * lighting;
    
    return float4(finalColor, 1.0f);
}
