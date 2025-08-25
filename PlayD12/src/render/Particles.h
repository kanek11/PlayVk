#pragma once

#include "PCH.h"

#include "Math/MMath.h"


// 
struct alignas(16) Particle
{
    Float3 pos;     // 12
    float  life;    // 16   
    Float3 vel;     // 28
    float  size;    // 32
    Float4 color;   // 48
    // 48 bytes
};

struct SimParams
{
    //float dt;
    //Float3 gravity;    
    uint32_t  maxParticles;  
    float spawnRate;    // /s
    Float2 lifeRange;   // {min, max}
    Float2 sizeRange;   // {min, max}
    Float3 emitterPos;
    float  initSpeed;   
    //uint   frameIndex;  //  
    //Float3 _pad;
};

//struct DrawParams
//{
//    Float4x4 viewProj;
//    Float2   viewportSize;
//    Float2   _pad0;
//};

struct FEmitterDesc {
    uint32_t maxParticles = 65536;
    float    spawnRate = 2000.f;
    Float3 emitterPos = { 0,0,0 };
    Float3 gravity = { 0,-9.8f,0 };
    Float2 lifeRange = { 1.0f, 2.0f };
    Float2 sizeRange = { 0.02f, 0.06f };
    float    initSpeed = 5.0f;
};

class Emitter {
public:
    explicit Emitter(const FEmitterDesc& desc) : m_desc(desc) {}
    void UpdateSimCB(SimParams& p, float dt, uint32_t frameIndex) {
        p.dt = dt;
        p.gravity = { m_desc.gravity.x, m_desc.gravity.y, m_desc.gravity.z };
        p.maxParticles = m_desc.maxParticles;
        p.spawnRate = m_desc.spawnRate;
        p.lifeRange = { m_desc.lifeRange.x, m_desc.lifeRange.y };
        p.sizeRange = { m_desc.sizeRange.x, m_desc.sizeRange.y };
        p.emitterPos = { m_desc.emitterPos.x, m_desc.emitterPos.y, m_desc.emitterPos.z };
        p.initSpeed = m_desc.initSpeed;
        p.frameIndex = frameIndex;
    }
private:
    FEmitterDesc m_desc;
};

//struct GPUParticleSpriteMaterial {
//    //  Texture SRV, sampler, blend state...
//    bool additive = false;
//};
//
//class GPUParticleSpriteRenderer {
//public:
//    void Draw(/*cmd, SRVs, CBVs, PSO...*/, uint32_t maxParticles) {
//        //  DrawInstanced(4, maxParticles)
//    }
//};

class GPUParticleSystem {
public:
    //GPUParticleSystem(GPUParticleEmitter emitter, GPUParticleSpriteRenderer renderer)
    //    : m_emitter(std::move(emitter)), m_renderer(std::move(renderer)) {
    //}

    void Tick(float dt) {
        //  SimParams、 CS、barrier、 
    }
private:
    //GPUParticleEmitter         m_emitter;
    //GPUParticleSpriteRenderer  m_renderer;
	//what ever else...
};
