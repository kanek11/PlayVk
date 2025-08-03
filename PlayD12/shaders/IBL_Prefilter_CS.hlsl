

#include "PBR/IBL_Common.hlsli"

TextureCube<float4> envMap : register(t0);
RWTexture2DArray<float4> prefilterMap : register(u0);

SamplerState linearWrapSampler : register(s0);

cbuffer prefilterCB : register(b1)
{
    uint Resolution;  
    uint NumSamples; // 1024-2048
    float Roughness;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    uint face = DTid.z;
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5f) / float(Resolution);
    float3 R = CubeMapTexelToDirection(uv, face);

    float3 N = R;
    float3 prefiltered = 0;
    float totalWeight = 0;

    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = normalize(2 * dot(R, H) * H - R);
        float NoL = saturate(dot(N, L));
        if (NoL > 0)
        {
            float3 sample = envMap.SampleLevel(linearWrapSampler, L, 0).rgb;
            sample = min(sample, float3(10.0, 10.0, 10.0)); // tweak value depending on HDR range
            prefiltered += sample * NoL;
            //prefiltered += envMap.SampleLevel(linearWrapSampler, L, 0).rgb * NoL;
            totalWeight += NoL;
        }
    }
    prefiltered = prefiltered / max(totalWeight, 0.0001);
    prefilterMap[uint3(coord, face)] = float4(prefiltered, 1.0f);
}