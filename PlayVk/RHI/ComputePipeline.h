#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"
 

namespace ParticleSystem
{
    inline const uint32_t PARTICLE_COUNT = 1;

    struct alignas(16) ParticleSSBO {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 velocity;
    };

    struct SystemInputUBO
    {
        float deltaTime{};
    };
}
using namespace ParticleSystem;



class FVkComputePipeline {
public:
    ~FVkComputePipeline()
    {
		shutdown(); 
    } 

    void init(); 
	void shutdown();

    virtual void CreateSceneResources();
    virtual void UpdateDescriptorSets();

    void CreatePipeline();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer); 
    void Compute(std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>&); 
    void UpdateInputs(SystemInputUBO deltaTime); 

    void CreateCommandBuffers(); 
	void CreateSyncObjects(); 
    
    
    WeakPtr<FVkDevice> deviceRef; 
    WeakPtr<FVkCommandContext> commandContextRef;
	WeakPtr<FVkHeapManager> heapManagerRef;


    SharedPtr<FVkShaderMap> shaderMap;  

    std::vector<SharedPtr<FVkBuffer>> systemInputUBOs;
	std::vector<SharedPtr<FVkBuffer>> particleSSBOs;


    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> computeCommandBuffers;
      
    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeInFlightFences; 


    int currentFrame{ 0 };
};
 