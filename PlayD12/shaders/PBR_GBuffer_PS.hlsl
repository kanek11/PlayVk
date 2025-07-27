
#include "Common/Math.hlsli"
#include "PBR/Gbuffer_Common.hlsli"
 
#include "PBR/PBR_Material.hlsli"


GBufferOutput PSMain(VSOutput input)
{
    GBufferOutput output;
     
    float3 worldNormal = input.WorldNormal;
    float3 worldTangent = input.WorldTangent;

    //
    float3 normal_TS = normalMap.Sample(linearWrapSampler, input.UV).rgb ; 
    normal_TS = UnpackRGB(normal_TS);
    
    float3x3 TBN = BuildTBN(worldNormal, worldTangent, input.TangentSign);
    worldNormal = TangentToWorld(TBN, normal_TS); 
     
    
    // 
    float3 albedo = GetBaseColor(input.UV);
    float metallic = GetMetallic(input.UV);
    float roughness = GetRoughness(input.UV);
    float ao = GetAO(input.UV);

    
    // GBuffer packing:
    output.rt0_albedo_ao = float4(albedo, ao);
    output.rt1_normal_rough = float4(worldNormal * 0.5f + 0.5f, roughness); // normal packed into [0,1]
    output.rt2_metallic_misc = float4(metallic, 0.0f, 0.0f, 0.0f); 

    return output;
}
