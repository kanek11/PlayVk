
#include "PBR/IBL_Common.hlsli" 

// Equirectangular direction to [0,1]^2 UV
float2 EquirectUV(float3 dir)
{
    float phi = atan2(dir.z, dir.x); // [-PI, PI]
    float theta = acos(clamp(dir.y, -1.0, 1.0)); // [0, PI]
    float u = phi / (2.0 * 3.14159265) + 0.5;
    float v = theta / 3.14159265;
    return float2(u, v);
}

Texture2D<float4> equirectHDR : register(t0);
SamplerState linearWrapSampler : register(s0);
RWTexture2DArray<float4> cubeMap : register(u0);

//cbuffer CubemapCB : register(b0)
//{
//    uint cubeSize;  
//};

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint x = DTid.x;
    uint y = DTid.y;
    uint face = DTid.z; // [0,5]

    uint cubeSize = 256;
    if (x >= cubeSize || y >= cubeSize || face >= 6)
        return;

    float2 uv = (float2(x, y) + 0.5) / float(cubeSize); // [0,1] center
    float3 dir = CubeMapTexelToDirection(uv, face);

    float2 eqUV = EquirectUV(dir);

    //so it won't overflow
    eqUV = saturate(eqUV);

    float4 color = equirectHDR.SampleLevel(linearWrapSampler, eqUV, 0);

    cubeMap[uint3(x, y, face)] = color;
}
