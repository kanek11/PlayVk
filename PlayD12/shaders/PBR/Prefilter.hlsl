#include "../Common/IBLCommon.hlsli"

RWTexture2DArray<float4> OutPrefilter : register(u0);
TextureCube EnvMap : register(t0);
SamplerState EnvSampler : register(s0);

cbuffer Params : register(b0)
{
    uint Resolution;
    uint NumSamples;
    float Roughness;
    uint MipLevel;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    uint face = DTid.z;
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5) / float(Resolution);
    float3 R = CubeMapTexelToDirection(uv, face);

    float3 N = R;
    float3 prefilteredColor = 0;
    float totalWeight = 0;

    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = normalize(2 * dot(R, H) * H - R);
        float NoL = max(dot(N, L), 0.0);
        if (NoL > 0)
        {
            prefilteredColor += EnvMap.SampleLevel(EnvSampler, L, 0).rgb * NoL;
            totalWeight += NoL;
        }
    }
    prefilteredColor = prefilteredColor / max(totalWeight, 0.0001);
    OutPrefilter[uint3(coord, face)] = float4(prefilteredColor, 1);
}
