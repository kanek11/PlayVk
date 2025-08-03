#ifndef PBR_IBL_HLSLI
#define PBR_IBL_HLSLI

#include "BRDF.hlsli"
#include "../Common/Samplers.hlsli"

TextureCube irradianceMap : register(t1, space2);
TextureCube prefilterMap : register(t2, space2);
Texture2D brdfLUT : register(t3, space2); 

float3 IBL_Diffuse(float3 N)
{
    return irradianceMap.Sample(linearWrapSampler, N).rgb;
}


// Example: IBL Specular from Prefiltered Env and BRDF LUT
float3 IBL_Specular(float3 F0, float roughness, float3 N, float3 V)
{ 
    float3 R = reflect(-V, N);
    float NdotV = saturate(dot(N, V));
    float3 F = FresnelSchlickRoughness(NdotV, F0, roughness);

    float MAX_MIP = 5.0; //roughness * MAX_MIP
    float3 prefiltered = prefilterMap.SampleLevel(linearWrapSampler, R, roughness * MAX_MIP).rgb;
    float2 brdf = brdfLUT.Sample(linearWrapSampler, float2(saturate(dot(N, V)), roughness)).rg;
    float3 specIBL = prefiltered * (F * brdf.x + brdf.y);
    
    return specIBL;
}

float2 IBL_BRDFLUT(float roughness, float3 N, float3 V ) 
{
    float NdotV = saturate(dot(N, V));
    return brdfLUT.Sample(linearWrapSampler, float2(saturate(NdotV), roughness)).rg;
}

 
float3 IBL_KD(float3 F0, float roughness, float3 N, float3 V, float metallic)
{
    float NdotV = saturate(dot(N, V));
    float3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    
    return kD;
}

#endif