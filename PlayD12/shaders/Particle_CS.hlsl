
#include "Common/Scene.hlsli"
#include "Particle_Common.hlsli"

RWStructuredBuffer<Particle> gParticles : register(u0);

//  uint counter, updated by cpu side
RWByteAddressBuffer gEmitCounter : register(u1);

cbuffer SimCB : register(b0)
{ 
    uint g_maxParticles;
    //float g_spawnRate; // 可保留/也可忽略，现以 gEmitCounter 为准Emit(rate * dt)
    float2 g_lifeRange;
    float2 g_sizeRange;
    float g_initSpeed; 
    
    float3 g_emitterPos;
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
float Rand01(uint s)
{
    return (Hash(s) & 0x00FFFFFF) / 16777216.0f;
}

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint i = id.x;
    if (i >= g_maxParticles)
        return;

    Particle p = gParticles[i];
    
    float3 gravity = float3(0, -9.81f, 0);
    float dt = gDeltaTime;
    float frameIndex = gTime / dt;

    if (p.life > 0.0f)
    {
        // integrate, life , fade
        p.vel += gravity * dt;
        p.pos += p.vel * dt;
        p.life -= dt;

        float maxLife = max(g_lifeRange.x, g_lifeRange.y);
        float fadeT = saturate(p.life / max(1e-3, maxLife * 0.2f));
        p.color.a = fadeT;
    }
    else
    { 
        uint old;
        // current value
        gEmitCounter.InterlockedAdd(0, 0, old); 
        if (old > 0)
        {
            //try get a slot and decrease counter
            uint old2;
            gEmitCounter.InterlockedAdd(0, (uint) (-1), old2); 
            if (old2 > 0)
            {
                // 真正生成
                uint seed = Hash(i + frameIndex * 9781u);
                float life = lerp(g_lifeRange.x, g_lifeRange.y, Rand01(seed + 3u));
                float size = lerp(g_sizeRange.x, g_sizeRange.y, Rand01(seed + 5u));

                // 半球方向
                float phi = 6.2831853f * Rand01(seed + 7u);
                float z = Rand01(seed + 11u) * 0.8f + 0.2f;
                float r = sqrt(saturate(1.0f - z * z));
                float3 dir = float3(r * cos(phi), z, r * sin(phi));

                p.pos = g_emitterPos;
                p.vel = dir * g_initSpeed;
                p.life = life;
                p.size = size;
                p.color = float4(1, 1, 1, 1);
            }
            // else: no available quota;
        }
        //else: no more emit quota 
    }

    gParticles[i] = p;
}
