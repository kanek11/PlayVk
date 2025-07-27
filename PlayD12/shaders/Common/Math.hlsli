#ifndef COMMON_MATH_HLSLI
#define COMMON_MATH_HLSLI

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

#endif



 

 
