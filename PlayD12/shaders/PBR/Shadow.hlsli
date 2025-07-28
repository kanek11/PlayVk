#ifndef PBR_MATERIAL_HLSLI
#define PBR_MATERIAL_HLSLI


Texture2D shadowMap : register(t0, space0);

SamplerState shadowMapSampler : register(s1);


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
        float shadowMapDepth = shadowMap.Sample(shadowMapSampler, shadowCoord.xy).r;
        float bias = 0.001f; // Tweak 
        inShadow = (shadowCoord.z - bias) > shadowMapDepth;
    }
    
    return inShadow;
}

#endif