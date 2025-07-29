// SM 6.8, 8×8×6 32² – 128² 
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID)
{
    uint face = DTid.z;
    float2 uv = (DTid.xy + 0.5) / EnvDim;
    float3 N = TangentSpaceFromCubemapUV(uv, face);

    float3 irradiance = 0;
    const uint SAMPLE_COUNT = 1024;
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float3 L = ImportanceSampleCosHemisphere(i, N);
        irradiance += HDRMap.SampleLevel(LinearWrapS, WorldDirToCubeUV(L), 0).rgb
                      * saturate(dot(N, L));
    }
    irradiance *= PI / SAMPLE_COUNT;
    IrradianceCube[DTid] = float4(irradiance, 1);
}

