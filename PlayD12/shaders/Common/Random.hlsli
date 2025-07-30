 
#ifndef COMMON_RANDOM_HLSLI
#define COMMON_RANDOM_HLSLI



uint Hash13(float3 p)
{
    p = frac(p * 0.1031f);
    p += dot(p, p.yzx + 19.19f);
    return frac((p.x + p.y) * p.z) * 65536;
}

#endif