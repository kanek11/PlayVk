 
 
#include "PBR/IBL_Common.hlsli"

TextureCube<float4> envMap : register(t0);
RWTexture2DArray<float4> irradianceMap : register(u0);

SamplerState linearSampler : register(s0); 

//cbuffer Params : register(b0)
//{
//    uint Resolution; // 32
//    uint NumSamples; // 1024
//}
 
float3 ImportanceSampleCosHemisphere(float2 Xi, float3 N)
{
    float phi = 2.0 * 3.1415926 * Xi.x;
    float cosTheta = sqrt(1.0 - Xi.y);
    float sinTheta = sqrt(Xi.y);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // TBN~ GGX
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);
    return normalize(tangentX * H.x + tangentY * H.y + N * H.z);
}


[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    
    uint Resolution = 32;  
    uint NumSamples = 1024; 
    
    uint face = DTid.z;
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5f) / float(Resolution);
    float3 N = CubeMapTexelToDirection(uv, face); 

    float3 irradiance = 0;
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 L = ImportanceSampleCosHemisphere(Xi, N); 
        float NoL = max(dot(N, L), 0.0f);
        irradiance += envMap.SampleLevel(linearSampler, L, 0).rgb * NoL;
    }
    irradiance = 3.1415926 * irradiance / NumSamples;
    irradianceMap[uint3(coord, face)] = float4(irradiance, 1.0f);
}

