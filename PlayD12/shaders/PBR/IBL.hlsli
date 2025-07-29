float3 PrefilterEnvMap(TextureCube env, float3 R, float roughness);
float3 SampleBRDFLUT(Texture2D lut, float NoV, float roughness);

TextureCube Irradiance : register(BINDING_IRRADIANCE_CUBE, SPACE_IBL);
TextureCube Prefiltered : register(BINDING_PREFILTER_CUBE, SPACE_IBL);
Texture2D BRDFLUT : register(BINDING_BRDF_LUT, SPACE_IBL);
SamplerState LinearClampS : register(s2, SPACE_IBL);





// GGX Importance Sampling
float3 ImportanceSampleGGX(float2 Xi, float roughness, float3 N)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // Transform to world space
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);

    return tangentX * H.x + tangentY * H.y + N * H.z;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

// Example: IBL Specular from Prefiltered Env and BRDF LUT
float3 IBL_Specular(float3 R, float roughness, float3 N, float3 V,
    TextureCube Prefiltered, SamplerState samplerPrefilter,
    Texture2D BRDFLUT, SamplerState samplerLUT)
{
    float NoV = saturate(dot(N, V));
    const float MAX_MIP = 4.0;  
    float3 prefilteredColor = Prefiltered.SampleLevel(samplerPrefilter, R, roughness * MAX_MIP).rgb;
    float2 brdf = BRDFLUT.Sample(samplerLUT, float2(NoV, roughness)).rg;
    return prefilteredColor * brdf.x + brdf.y;
}
