#include "PBR/IBL_Common.hlsli"

Texture2D<float4> equirectHDR : register(t0);
RWTexture2DArray<float4> cubeMap : register(u0); 
SamplerState linearWrapSampler : register(s0); 

//cbuffer CubeMapCB : register(b0)
//{
//    uint FaceSize; 
//};

[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint x = DTid.x;
    uint y = DTid.y;
    uint faceIndex = DTid.z;
    
    uint FaceSize = CUBE_FACE_SIZE;

    if (x >= FaceSize || y >= FaceSize || faceIndex >= 6)
        return;

    // Compute normalized coords in [0,1]
    float2 uv = (float2(x, y) + 0.5f) / FaceSize;

    // Convert (uv, faceIndex) to direction vector
    float3 dir = CubeMapTexelToDirection(uv, faceIndex);

    // Convert direction vector to equirectangular UV coords
    float2 equirectUV;
    equirectUV.x = 0.5f + atan2(dir.z, dir.x) / (2.0f * 3.14159265f);
    equirectUV.y = 0.5f - asin(dir.y) / 3.14159265f;

    // Sample HDR texture
    float4 color = equirectHDR.Sample(linearWrapSampler, equirectUV);

    // Write output to cubemap face
    cubeMap[uint3(x, y, faceIndex)] = color;
}
