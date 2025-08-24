
#include "Common/Scene.hlsli" 

RWTexture2D<float4> Output : register(u0);

float sdCircle(float2 p, float r)
{
    return length(p) - r;
}

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * (1.0 / 4.0);
}

float3 shadeEye(float d)
{
    //falloff rate, higher = tight
    float glow = exp(-40.0 * max(d, 0.0));
    float core = smoothstep(0.0, 0.01, -d);
    return float3(glow + core, glow, core * 0.5 + glow * 0.2);
}

float blinkPhase(float t, float waitTime, float blinkTime)
{
    // cycle length = wait + blink
    float cycle = waitTime + blinkTime;

    // where we are in the cycle [0..cycle)
    float phase = fmod(t, cycle);

    // if we're in the wait zone -> open
    if (phase < waitTime)
        return 1.0;

    // normalize [0..1] across the blink duration
    float b = (phase - waitTime) / blinkTime;

    // symmetric open ¨ closed ¨ open using abs
    return abs(2.0 * b - 1.0);
}


[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    uint width, height;
    Output.GetDimensions(width, height);

    float2 uv = (coord + 0.5) / float2(width, height);
    float2 p = uv * 2.0 - 1.0;

    //position
    float2 leftEye = float2(-0.12, 0.0);
    float2 rightEye = float2(0.12, 0.0);

    // === Blink
    //float period = 1.0;  //short means faster
    //float t = frac(gTime / period); // 0~1
    //float blinkPhase = abs(2.0 * t - 1.0); // 1 open, 0 close

    // Map phase to vertical scale: 
    // blinkPhase=1 ~ scaleY=1 circle, 
    // blinkPhase=0 ~ scaleY=0 flat line  
    // 5s wait, 0.5s blink cycle
    
    float baseAspect = 1.8; //smallr makes eyes "flatter"
    
    float blinkY = blinkPhase(gTime, 5.0, 0.5);
    blinkY = max(blinkY, 0.05); // avoid divide by zero
    
    float scaleY = baseAspect * blinkY; 

    // Apply Y scaling to simulate eyelids closing
    float2 pL = (p - leftEye) * float2(1.0, 1.0 / scaleY);
    float2 pR = (p - rightEye) * float2(1.0, 1.0 / scaleY);

    //arg2 for size
    float dL = sdCircle(pL, 0.05);
    float dR = sdCircle(pR, 0.05);

    float d = smin(dL, dR, 0.2);

    float3 col = shadeEye(d); 
    
    float alpha = length(col) > 0.01f ? 1.0f : 0.0f;

    Output[coord] = float4(col, alpha);
}
