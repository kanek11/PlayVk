#ifndef PBR_SHADING_HLSLI
#define PBR_SHADING_HLSLI

Texture2D rt0_albedo_ao : register(t0, space1);
Texture2D rt1_normal_rough : register(t1, space1);
Texture2D rt2_position_metallic: register(t2, space1);
 
SamplerState linearWrapSampler : register(s0);


#endif 