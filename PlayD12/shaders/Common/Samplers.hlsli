
#ifndef SAMPLER_HLSLI
#define SAMPLER_HLSLI

SamplerState linearWrapSampler : register(s0, space0);
 
SamplerState depthSampler : register(s1, space0);

SamplerState pointClampSampler : register(s2, space0);
SamplerState pointWrapSampler : register(s3, space0);
#endif