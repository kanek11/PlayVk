cbuffer SceneCB : register(b0, space0)
{
    float4x4 PVMatrix;
    float4x4 LightPVMatrix;
    float3 LightDirection;
    float3 LightColor;
    
    float padding[26];
};
 
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    
    float4 lightSpacePos : TEXCOORD1;
};

Texture2D baseMap : register(t0);
SamplerState baseMapSampler : register(s0);

Texture2D shadowMap : register(t1);
SamplerState shadowMapSampler : register(s1);
 
float4 PSMain(PSInput input) : SV_TARGET
{
    //debug:
    float2 viewportSize = float2(2048.f, 2048.f); // Example viewport size, replace with actual viewport size if available
    float2 debugUV = input.position.xy / viewportSize;
     
    float depth = shadowMap.Sample(shadowMapSampler, debugUV).r;
    //return float4(depth.xxx, 1.0f); 
    
    
    // Transform from clip space to texture UV
    float3 shadowCoord = input.lightSpacePos.xyz / input.lightSpacePos.w;
    shadowCoord = shadowCoord * 0.5f + 0.5f; // [-1,1] to [0,1]
    
        // Early out if outside light frustum
    bool inShadow = true;
    if (shadowCoord.x >= 0.0f && shadowCoord.x <= 1.0f &&
        shadowCoord.y >= 0.0f && shadowCoord.y <= 1.0f &&
        shadowCoord.z >= 0.0f && shadowCoord.z <= 1.0f)
    {
        float shadowMapDepth = shadowMap.Sample(shadowMapSampler, shadowCoord.xy).r;
        float bias = 0.005f; // Tweak for acne removal
        //inShadow = (shadowCoord.z - bias) > shadowMapDepth;
        
        inShadow = (shadowCoord.z ) > shadowMapDepth;
    }
    
    
    // Sample texture color
    float4 texColor = baseMap.Sample(baseMapSampler, input.texCoord);

    // Basic Lambert lighting
    float NdotL = max(dot(input.normal, LightDirection), 0.0f);
    float ambient = 0.3f;

    float3 diffuse = LightColor * NdotL;
    float3 lighting = ambient + diffuse;

    // Multiply texture color by vertex color and lighting
    //float3 finalColor = input.color.rgb * lighting;
    //float3 finalColor = lighting * input.color.rgb;
    //float3 finalColor = (texColor.xyz * lighting) * depth.xxx;
    float3 finalColor = (texColor.xyz * lighting) * (1 - inShadow);
    
  
    return float4(finalColor, 1.0f);
   //return float4(float3(depth, depth, depth), 1.0f);
   // return float4(float3(inShadow, inShadow, inShadow), 1.0f);
    //return float4(shadowCoord, 1.0f);
}