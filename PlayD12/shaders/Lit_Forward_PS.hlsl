 
 
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

Texture2D baseMap : register(t0);
SamplerState baseMapSampler : register(s0);

Texture2D shadowMap : register(t1);
SamplerState shadowMapSampler : register(s1);


static const float3 lightDirection = normalize(float3(0.577f, 0.277f, 0.377f)); // Example light dir
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f); // White light



float4 PSMain(PSInput input) : SV_TARGET
{
    //debug:
    float2 viewportSize = float2(1280.0f, 720.0f); // Example viewport size, replace with actual viewport size if available
    float2 debugUV = input.position.xy / viewportSize;
     
    float depth = shadowMap.Sample(shadowMapSampler, debugUV).r;
    //return float4(depth.xxx, 1.0f); 
    
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
    float3 finalColor = (texColor.xyz * lighting) * depth.xxx;
    
    return float4(finalColor, 1.0f); 
    //return float4(float3(depth, depth, depth), 1.0f);
}