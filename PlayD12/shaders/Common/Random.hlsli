 
#ifndef COMMON_RANDOM_HLSLI
#define COMMON_RANDOM_HLSLI



uint Hash13(float3 p)
{
    p = frac(p * 0.1031f);
    p += dot(p, p.yzx + 19.19f);
    return frac((p.x + p.y) * p.z) * 65536;
}


uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
float Rand01(uint seed)
{
    return (Hash(seed) & 0x00FFFFFF) / 16777216.0f;
}
float2 Rand2(uint seed)
{
    return float2(Rand01(seed), Rand01(seed * 1664525u + 1013904223u));
}

#endif