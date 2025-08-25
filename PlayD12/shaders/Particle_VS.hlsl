// VS_PS_Sprite.hlsl
#include "Common/Scene.hlsli"

#include "Particle_Common.hlsli"

StructuredBuffer<Particle> gParticles : register(t0);

struct VSOut
{
    float4 posH : SV_Position;
    float2 uv : TEXCOORD0;
    float4 col : COLOR0;
};

static const float2 kCorners[4] =
{
    float2(-0.5, -0.5),
    float2(0.5, -0.5),
    float2(-0.5, 0.5),
    float2(0.5, 0.5),
};

VSOut VSMain(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VSOut output;
    Particle p = gParticles[iid];
     
    if (p.life <= 0.0f || p.color.a <= 0.001f)
    {
        output.posH = float4(0, 0, 0, 0);
        output.uv = float2(0, 0);
        output.col = 0;
        return output;
    }

    float2 corner = kCorners[vid];
    float3 camRight = normalize(gInvView[0].xyz);
    float3 camUp = normalize(gInvView[1].xyz);

    float3 worldPos = p.pos + (camRight * corner.x + camUp * corner.y) * p.size;
    float4 clipPos = mul(gPVMatrix, float4(worldPos, 1));

    output.posH = clipPos;
    output.uv = corner * float2(1, -1) + float2(0.5, 0.5);
    output.col = p.color;
    return output;
}


