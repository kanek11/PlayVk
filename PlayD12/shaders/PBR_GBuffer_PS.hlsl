
#include "Common/Math.hlsli"
#include "PBR/Gbuffer_Common.hlsli"
 
#include "PBR/PBR_Material.hlsli"


GBufferOutput PSMain(VSOutput input)
{
    GBufferOutput output;
     
    float3 worldNormal = input.WorldNormal;
    float3 worldTangent = input.WorldTangent;

    //
    if (gMaterial.useNormalMap)
    {
        float3 normal_TS = normalMap.Sample(linearWrapSampler, input.UV).rgb;
        normal_TS = DecodeRGB(normal_TS);
    
        //normal_TS = float3(0.0f, 0.0f, 1.0f);
        float3x3 TBN = BuildTBN(worldNormal, worldTangent, input.TangentSign);
        worldNormal = TangentToWorld(TBN, normal_TS); 
    } 
    
    // 
    float3 albedo = GetBaseColor(input.UV);
    float metallic = GetMetallic(input.UV);
    float roughness = GetRoughness(input.UV);
    float ao = GetAO(input.UV);

    
    // GBuffer packing:
    output.rt0_albedo_ao = float4(albedo, ao);
    output.rt1_normal_rough = float4(EncodeRGB(worldNormal), roughness); // normal packed into [0,1]
 
    //output.rt1_normal_rough = float4(EncodeRGB(input.WorldNormal), roughness); // normal packed into [0,1]
    //output.rt1_normal_rough = float4(EncodeRGB(input.WorldTangent), roughness); // normal packed into [0,1]
    output.rt2_position_metallic = float4(input.WorldPos, metallic);

    return output;
}