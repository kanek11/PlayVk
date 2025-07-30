

#include "PBR/BRDF.hlsli"
#include "PBR/IBL_Common.hlsli"
   
//RG=integral(A,B)  (Smith GGX, Schlick F)
RWTexture2D<float2> brdfLUT : register(u0);

//optional 
//cbuffer Params : register(b0)
//{
//    uint Resolution; // eg: 512
//    uint NumSamples; // eg:1024
//};

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    
    uint Resolution = 1024;
    uint NumSamples = 1024;  
    
    if (coord.x >= Resolution || coord.y >= Resolution)
        return;

    float2 uv = (coord + 0.5) / float(Resolution);
    float NdotV = uv.x;
    float roughness = uv.y;

    float3 V = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float A = 0, B = 0;
    [loop]
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, roughness, float3(0, 0, 1));
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));

        if (NoL > 0)
        {
            float G = GeometrySmith2(NdotV, NoL, roughness);
            float G_Vis = (G * VoH) / (NoH * NdotV + 1e-5);
            float Fc = pow(1.0 - VoH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= NumSamples;
    B /= NumSamples;
    
    brdfLUT[coord] = float2(A, B);
}
