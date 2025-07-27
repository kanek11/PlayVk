#include "Common/SpaceBindings.hlsli"
#include "Common/Math.hlsli"
#include "PBR/GBuffer_Common.hlsli"       
#include "PBR/PBR_Material.hlsli"         


 


float4 PSMain(VS_OUT i) : SV_Target0
{
    float2 uv = i.uv;

    //---
    float4 albMet = gAlbedoMetal.Sample(gLinearClamp, uv);
    float4 nrmRgh = gNormalRoughness.Sample(gLinearClamp, uv);
    float4 aoEmit = gAOEmissive.Sample(gLinearClamp, uv);
    float depth = gDeviceDepth.Load(int3(uint2(uv * gAlbedoMetal.GetDimensions()), 0)).r;

    float3 albedo = albMet.rgb;
    float metallic = albMet.a;

    float3 N = DecodeNormal(nrmRgh.xyz);
    float rough = nrmRgh.w;

    float ao = aoEmit.r;
    float emissiveStrength = aoEmit.g;
    float3 emissive = aoEmit.bgr * emissiveStrength;

    float3 worldPos = ReconstructWorldPos(uv, depth);
    float3 V = normalize(gCameraWS - worldPos);

    //lighting loop
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

    float3 Lo = 0.0f;
    [loop]
    for (uint l = 0; l < gActiveLightCount; ++l)
    {
        DirectionalLight light = gLights[l];
        float3 L = normalize(-light.directionWS);
        float3 H = normalize(V + L);

        float NdotL = NdotLClamp(N, L);
        if (NdotL <= 0.0f)
            continue;

        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
        float D = DistributionGGX(N, H, rough);
        float G = GeometrySmith(N, V, L, rough);

        float3 spec = (D * F * G) / max(4.0f * NdotL * max(dot(N, V), 0.001f), 0.001f);
        float3 kS = F;
        float3 kD = (1.0f - kS) * (1.0f - metallic);

        float3 irradiance = light.color * light.intensity * NdotL;
        Lo += (kD * albedo / PI + spec) * irradiance;
    }

    // --- Ambient / IBL hook ------------------------------------------------------
    float3 ambient = albedo * ao; // replace with proper IBL if available

    float3 color = ambient + Lo + emissive;

    // --- Simple ACES?approx tonemap + gamma --------------------------------------
    color = pow(color / (color + 1.0f), 1.0f / 2.2f);

    return float4(color, 1.0f);
}


