#include "Common/Scene.hlsli" 
#include "Common/Math.hlsli"
 
#include "PBR/PBR_Shading_Common.hlsli"
#include "PBR/BRDF.hlsli"   
#include "PBR/lighting.hlsli"

#include "PBR/Shadow.hlsli"


TextureCube skybox : register(t1, space0);


struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

float4 PSMain(VSOutput input) : SV_Target0
{
    float2 uv = input.texCoord;

    //---
    float4 alb_ao = rt0_albedo_ao.Sample(linearWrapSampler, uv);
    float4 norm_rough = rt1_normal_rough.Sample(linearWrapSampler, uv);
    float4 pos_metal = rt2_position_metallic.Sample(linearWrapSampler, uv);

    float3 albedo = alb_ao.rgb;
    float ao = alb_ao.a;
    
    float3 N = DecodeRGB(norm_rough.xyz);
    float rough = norm_rough.w;
     
    float3 worldPos = pos_metal.xyz;
    float metallic = pos_metal.a;

    float3 V = normalize(gCameraPos - worldPos);
     
     
    //mimic light
    DirectionalLight light;
    light.direction = gLightDir;
    light.color = gLightColor;
    light.intensity = gLightIntensity * 5; 
    
    float4 lightSpacePos = mul(gLightPVMatrix, float4(worldPos.xyz , 1.0f));
    bool inShadow = isInShadow(lightSpacePos);
    
    
    
    //lighting loop
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic); 

    float3 Lo = 0.0f;
    //[loop]
    //for (uint l = 0; l < gActiveLightCount; ++l)
    //{ 
    for (uint l = 0; l < 1; ++l)
    {
        //DirectionalLight light = gLights[l]; 
        float3 L = normalize(light.direction);
        float3 H = normalize(V + L);

        float NdotL = saturate(dot(N, L));
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
        //Lo += (spec) * irradiance;
    }

    //Ambient / IBL hook ,replace with proper IBL if available,
    // + emissive if avaliable;
    //float3 ambient = albedo * ao; 
    float3 ambient = albedo * 0.2 * ao;
    float3 color = ambient;
    if(!inShadow)
        color += Lo;
    
    //UVToViewDir(uv);
    //float3 worldDir = normalize(-worldPos);
    float viewDepth = ds_viewDepth.Sample(linearWrapSampler, uv).r;
    if(viewDepth > 0.9999f)
    { 
        float3 worldDir = UVToViewDir(uv);
        float4 skyboxColor = skybox.Sample(linearWrapSampler, worldDir);
        color = skyboxColor.rgb;
    }

    //color = skyboxColor.xyz;
    //color = worldDir;
    //color = float3(uv.x, uv.y, 0.0f);
    //Simple ACES approx tonemap + gamma 
    //color = pow(color / (color + 1.0f), 1.0f / 2.2f);

    //return float4(EncodeRGB(N), 1.0f);
    return float4(color, 1.0f);
}

