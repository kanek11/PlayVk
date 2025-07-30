#ifndef COMMON_MATH_HLSLI
#define COMMON_MATH_HLSLI

static const float PI = 3.1415926535897932f;

 
/// [0,1] to [-1,1] 
inline float3 DecodeRGB(float3 color)
{
    return color * 2.0f - 1.0f;
}

inline float3 EncodeRGB(float3 color)
{
    return color * 0.5f + 0.5f;
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
    return normalize(mul(transpose(TBN), n_TS));
}

inline float3 WorldToTangent(float3x3 TBN, float3 n_WS)
{
    return normalize(mul(TBN, n_WS));
}
 


#endif



 

 