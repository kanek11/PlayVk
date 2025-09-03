#ifndef PBR_MATERIAL_HLSLI
#define PBR_MATERIAL_HLSLI

#include "../Common/Samplers.hlsli"
Texture2D shadowMap : register(t0, space0);

bool isInShadow(float4 lightSpacePos)
{
    // Transform from clip space to texture UV
    float3 shadowCoord = lightSpacePos.xyz / lightSpacePos.w;
    shadowCoord.xy = shadowCoord.xy * 0.5f + 0.5f; // [-1,1] to [0,1]  
    //flip y: 
    shadowCoord.xy = float2(shadowCoord.x, 1.0f - shadowCoord.y); // Flip Y coordinate for texture sampling
    
   // Early out if outside light frustum
    bool inShadow = false;
    if (shadowCoord.x >= 0.0f && shadowCoord.x <= 1.0f &&
        shadowCoord.y >= 0.0f && shadowCoord.y <= 1.0f &&
        shadowCoord.z >= 0.0f && shadowCoord.z <= 1.0f)
    {
        float shadowMapDepth = shadowMap.Sample(depthSampler, shadowCoord.xy).r;
        float bias = 0.001f; // Tweak 
        inShadow = (shadowCoord.z - bias) > shadowMapDepth;
    }
    
    return inShadow;
}


// Instead of returning bool, return shadow factor
float getShadowFactor(float4 lightSpacePos)
{
    float3 shadowCoord = lightSpacePos.xyz / lightSpacePos.w;
    shadowCoord.xy = shadowCoord.xy * 0.5f + 0.5f;
    shadowCoord.xy = float2(shadowCoord.x, 1.0f - shadowCoord.y);

    if (shadowCoord.x < 0.0f || shadowCoord.x > 1.0f ||
        shadowCoord.y < 0.0f || shadowCoord.y > 1.0f ||
        shadowCoord.z < 0.0f || shadowCoord.z > 1.0f)
    {
        return 1.0f; // fully lit if outside frustum
    }

    float bias = 0.00723f;
    float2 texelSize = 1.0f / float2(4096.0f,4096.0f);
    float shadow = 0.0f;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += shadowMap.SampleCmpLevelZero(shadowPCFSampler, shadowCoord.xy + offset, shadowCoord.z - bias);
        }
    } 
    shadow /= 9.0f; // average 

    return shadow; // 0.0 = fully shadowed, 1.0 = fully lit
}


#endif