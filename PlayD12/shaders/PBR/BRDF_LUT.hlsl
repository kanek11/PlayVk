#include "../Common/IBLCommon.hlsli"

RWTexture2D<float2> OutBRDFLUT : register(u0);

cbuffer Params : register(b0)
{
    uint Resolution;
    uint NumSamples;
};

float GeometrySmith(float NoV, float NoL, float roughness)
{
    float a = roughness * roughness;
    float G1V = NoV / (NoV * (1.0 - a) + a);
    float G1L = NoL / (NoL * (1.0 - a) + a);
    return G1V * G1L;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5) / float(Resolution);
    float NdotV = uv.x;
    float roughness = uv.y;

    float3 V = float3(sqrt(1 - NdotV * NdotV), 0, NdotV);
    float A = 0, B = 0;

    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, roughness, float3(0, 0, 1));
        float3 L = normalize(2 * dot(V, H) * H - V);

        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));

        if (NoL > 0)
        {
            float G = GeometrySmith(NdotV, NoL, roughness);
            float G_Vis = (G * VoH) / (NoH * NdotV);
            float Fc = pow(1.0 - VoH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= NumSamples;
    B /= NumSamples;
    OutBRDFLUT[coord] = float2(A, B);
}
