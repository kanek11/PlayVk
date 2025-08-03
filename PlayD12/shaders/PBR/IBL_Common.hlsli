#ifndef IBL_COMMON_HLSLI
#define IBL_COMMON_HLSLI


cbuffer IBL_CB : register(b0)
{
    uint CUBE_FACE_SIZE  ;
    uint IRRADIANCE_SIZE  ;
    uint IRRADIANCE_NUMSAMPLES ;
    uint PREFILTER_SIZE  ;
    uint PREFILTER_NUMSAMPLES ;
    uint BRDF_LUT_SIZE  ;
    uint BRDF_LUT_NUMSAMPLES ;
}; 

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


// uv [0,1], face [0,5]
float3 CubeMapTexelToDirection(float2 uv, uint face)
{
    float u = 2.0f * uv.x - 1.0f;
    float v = 2.0f * uv.y - 1.0f;

    float3 dir = 0;
    switch (face)
    {
        case 0:
            dir = float3(1, -v, -u);
            break; // +X
        case 1:
            dir = float3(-1, -v, u);
            break; // -X
        case 2:
            dir = float3(u, 1, v);
            break; // +Y
        case 3:
            dir = float3(u, -1, -v);
            break; // -Y
        case 4:
            dir = float3(u, -v, 1);
            break; // +Z
        case 5:
            dir = float3(-u, -v, -1);
            break; // -Z
    }
    return normalize(dir);
}

 
float3 ImportanceSampleGGX(float2 Xi, float roughness, float3 N)
{
    float a = roughness * roughness;
    float phi = 2.0 * 3.1415926535897932 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    //TBN
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);

    return normalize(tangentX * H.x + tangentY * H.y + N * H.z);
}


#endif