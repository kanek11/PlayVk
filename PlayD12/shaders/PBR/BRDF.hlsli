#ifndef BRDF_HLSLI
#define BRDF_HLSLI
  
 
//Fresnel term, Schlick approximation 
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    // cosTheta~dot(H, V)  clamp [0,1]
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 F90 = saturate(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness)); // or clamp to avoid issues
    return F0 + (max(F90, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
 
//NDF ~ GGX / Trowbridge Reitz
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness; //perceptual roughness alpha
    float a2 = a * a;

    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    //Denominator of GGX (avoid division by 0 with small epsilon)
    float denom = NdotH2 * (a2 - 1.0f) + 1.0f;
    return a2 / max(3.1415926535897932f * denom * denom, 1e-5f);
}

//Geometry/Visibility term~ Smith with Schlick-GGX lambda
float GeometrySchlickGGX(float NdotX, float roughness)
{
    //kappa = ( (alpha + 1)^ 2 ) / 8, see UE4/Disney BRDF 
    float r = roughness + 1.0f;
    float k = (r * r) * 0.125f;

    return NdotX / (NdotX * (1.0f - k) + k);
}



float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL; //height-correlated Smith
}


float GeometrySmith2(float NoV, float NoL, float roughness)
{
    float a = roughness * roughness;
    float k = (a * a) / 2.0f;
    float G_V = NoV / (NoV * (1.0 - k) + k);
    float G_L = NoL / (NoL * (1.0 - k) + k);
    return G_V * G_L;
}


#endif