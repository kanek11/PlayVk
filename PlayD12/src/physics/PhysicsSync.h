#pragma once

#include <functional>
#include <vector>
#include <mutex>

//#include "Gameplay/Components/PrimitiveComponent.h"

//using ActorId = Gameplay::FPrimitiveComponentId;
using ActorId = uint32_t;

using PhysicsCommand = std::function<void()>;


class PhysicsCommandBuffer {
public:
    PhysicsCommandBuffer() : m_writeIndex(0), m_readIndex(1) {}

    void Enqueue(PhysicsCommand cmd) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_commandBuffers[m_writeIndex].push_back(std::move(cmd));
    }

    void Execute() {
        auto& cmds = m_commandBuffers[m_readIndex];
        for (auto& cmd : cmds) {
            cmd();
        }
        cmds.clear();
    }

    void SwapBuffers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(m_writeIndex, m_readIndex);
    }

    void Clear() {
        m_commandBuffers[m_readIndex].clear();
        m_commandBuffers[m_writeIndex].clear();
    }

private:
    std::vector<PhysicsCommand> m_commandBuffers[2];
    int m_writeIndex;
    int m_readIndex;
    std::mutex m_mutex;
};


struct PhysicsTransform {
    Float3 position;
    DirectX::XMVECTOR rotation;
};

using PhysicsTransformBuffer = std::unordered_map<ActorId, PhysicsTransform>;

class PhysicsTransformSyncBuffer {
public:
    PhysicsTransformSyncBuffer() : m_writeIndex(0), m_readIndex(1) {}
      
    void MarkToRemove(ActorId actor) { 
        m_Buffers[m_writeIndex].erase(actor);
        removeList.push_back(actor);
    } 

    void Write(ActorId actor, const PhysicsTransform& transform) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_Buffers[m_writeIndex][actor] = transform;
    }

    void SwapBuffers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(m_writeIndex, m_readIndex);

        if (!removeList.empty()) {
            for (auto& id : removeList) {
                m_Buffers[m_writeIndex].erase(id);
                removeList.clear();
            }  
        }
    }

    PhysicsTransformBuffer& GetReadBuffer() {
        return m_Buffers[m_readIndex];
    }

    PhysicsTransformBuffer& GetWriteBuffer() {
        return m_Buffers[m_writeIndex];
    }
    void Clear() {
        m_Buffers[m_readIndex].clear();
        m_Buffers[m_writeIndex].clear();
    }

private:
    PhysicsTransformBuffer m_Buffers[2];
    int m_writeIndex;
    int m_readIndex;
    std::mutex m_mutex; 

    std::vector<ActorId> removeList;
};