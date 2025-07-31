#pragma once
#pragma once
#include <functional>
#include <vector>
#include <mutex>

 
    using RenderCommand = std::function<void()>;
 
    class RenderCommandBuffer {
    public:
        RenderCommandBuffer() : m_writeIndex(0), m_readIndex(1) {}
 
        void Enqueue(RenderCommand cmd) {
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
        std::vector<RenderCommand> m_commandBuffers[2]; //double buffering
        int m_writeIndex;
        int m_readIndex;
        std::mutex m_mutex;
    };

 
