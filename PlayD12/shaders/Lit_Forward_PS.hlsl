#include "Common/Scene.hlsli"
 
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    
    float4 lightSpacePos : TEXCOORD1;
};

Texture2D baseColorMap : register(t0);
SamplerState baseMapSampler : register(s0);

Texture2D shadowMap : register(t1);
SamplerState shadowMapSampler : register(s1);
 
float4 PSMain(PSInput input) : SV_TARGET
{
    // Transform from clip space to texture UV
    float3 shadowCoord = input.lightSpacePos.xyz / input.lightSpacePos.w;
    shadowCoord.xy = shadowCoord.xy * 0.5f + 0.5f; // [-1,1] to [0,1]  
    //flip y: 
    shadowCoord.xy = float2(shadowCoord.x, 1.0f - shadowCoord.y); // Flip Y coordinate for texture sampling
    
   // Early out if outside light frustum
    bool inShadow = false;
    if (shadowCoord.x >= 0.0f && shadowCoord.x <= 1.0f &&
        shadowCoord.y >= 0.0f && shadowCoord.y <= 1.0f &&
        shadowCoord.z >= 0.0f && shadowCoord.z <= 1.0f)
    {
        float shadowMapDepth = shadowMap.Sample(shadowMapSampler, shadowCoord.xy).r;
        float bias = 0.001f; // Tweak 
        inShadow = (shadowCoord.z - bias) > shadowMapDepth; 
    }
    
    
    // Sample texture color
    float4 texColor = baseColorMap.Sample(baseMapSampler, input.texCoord);

    // Basic Lambert lighting
    float NdotL = max(dot(input.normal, gLightDir), 0.0f);
    float ambient = 0.3f;

    float3 diffuse = gLightColor * NdotL;
    float3 lighting = ambient + diffuse;

    // Multiply texture color by vertex color and lighting
    //float3 finalColor = input.color.rgb * lighting;
    //float3 finalColor = lighting * input.color.rgb;
    //float3 finalColor = (texColor.xyz * lighting) * depth.xxx;
    float3 finalColor = (texColor.xyz * lighting) * (1 - inShadow);
    
  
    return float4(finalColor, 1.0f); 
}