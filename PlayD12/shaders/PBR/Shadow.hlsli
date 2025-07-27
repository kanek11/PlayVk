Texture2D gShadowMap : register(t5, space1);
SamplerComparisonState gShadowSampler : register(s4);

float SampleShadow(float4 lightSpacePos)
{
    float3 proj = lightSpacePos.xyz / lightSpacePos.w;
    return gShadowMap.SampleCmpLevelZero(
        gShadowSampler, proj.xy, proj.z - 0.001f); 
}

float SampleShadowPCF(Texture2D shadowMap, SamplerState s, float4 shadowCoord);
