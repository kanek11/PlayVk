#ifndef COMMON_MATH_HLSLI
#define COMMON_MATH_HLSLI

static const float PI = 3.1415926535f;



/// [0,1] to [-1,1] 
inline float3 UnpackRGB(float3 color)
{
    return color * 2.0f - 1.0f;
}  
 
// Gram-Schmidt
inline float3x3 BuildTBN(float3 N, float3 T, float sign)
{
    T = normalize(T - N * dot(N, T)); 
    float3 B = cross(N, T) * sign;
    return float3x3(T, B, N);
} 

inline float3 TangentToWorld(float3x3 TBN, float3 n_TS)
{
    return normalize(mul(TBN, n_TS));
} 

inline float3 WorldToTangent(float3x3 TBN, float3 n_WS)
{
    return normalize(mul(transpose(TBN), n_WS));
}




float3 ReconstructWorldPos(float2 uv, float deviceDepth)
{
    float4 clip = float4(uv * 2.0f - 1.0f, deviceDepth, 1.0f);
    float4 ws = mul(gInvViewProj, clip);
    return ws.xyz / ws.w;
}

float NdotLClamp(float3 N, float3 L)
{
    return saturate(dot(N, L));
}




#endif



 

 
