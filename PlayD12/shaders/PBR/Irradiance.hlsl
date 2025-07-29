#include "../Common/IBLCommon.hlsli"

RWTexture2DArray<float4> OutIrradiance : register(u0);
TextureCube EnvMap : register(t0);
SamplerState EnvSampler : register(s0);

cbuffer Params : register(b0)
{
    uint Resolution;
    uint NumSamples;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    uint face = DTid.z;
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5) / float(Resolution);
    float3 N = CubeMapTexelToDirection(uv, face); // 需实现CubeMapTexelToDirection

    float3 irradiance = 0;
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 L = ImportanceSampleGGX(Xi, 1.0, N);
        float NoL = max(dot(N, L), 0.0);
        irradiance += EnvMap.SampleLevel(EnvSampler, L, 0).rgb * NoL;
    }
    irradiance = PI * irradiance / NumSamples;
    OutIrradiance[uint3(coord, face)] = float4(irradiance, 1);
}