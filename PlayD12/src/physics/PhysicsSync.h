#pragma once

#include <functional>
#include <vector>
#include <mutex>


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

private:
    std::vector<PhysicsCommand> m_commandBuffers[2]; 
    int m_writeIndex;
    int m_readIndex;
    std::mutex m_mutex;
};


struct PhysicsTransfrom {
	Float3 position;
	DirectX::XMVECTOR rotation;
};;

using PhysicsTransformBuffer = std::unordered_map<ActorHandle, PhysicsTransfrom>;

class PhysicsTransformSyncBuffer {
public:
    PhysicsTransformSyncBuffer() : m_writeIndex(0), m_readIndex(1) {}

	void Write(ActorHandle actor, const PhysicsTransfrom& transform) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_Buffers[m_writeIndex][actor] = transform;
	}

    void SwapBuffers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(m_writeIndex, m_readIndex);
    }

	PhysicsTransformBuffer& GetReadBuffer() {
		return m_Buffers[m_readIndex];
	}

	PhysicsTransformBuffer& GetWriteBuffer() {
		return m_Buffers[m_writeIndex];
	}

private:
    PhysicsTransformBuffer m_Buffers[2];
    int m_writeIndex;
    int m_readIndex;
    std::mutex m_mutex;
};

