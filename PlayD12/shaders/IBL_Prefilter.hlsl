

#include "PBR/IBL_Common.hlsli"

TextureCube<float4> envMap : register(t0);
RWTexture2DArray<float4> prefilter : register(u0); 

SamplerState linearSampler : register(s0);

cbuffer Params : register(b0)
{
    uint Resolution; // 128/256/512
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
            prefiltered += envMap.SampleLevel(linearSampler, L, 0).rgb * NoL;
            totalWeight += NoL;
        }
    }
    prefiltered = prefiltered / max(totalWeight, 0.0001);
    prefilter[uint3(coord, face)] = float4(prefiltered, 1.0f);
}
