#pragma once

#include <vulkan/vulkan.h>
#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"




inline const uint32_t PARTICLE_COUNT = 1000;


struct alignas(16) ParticleSSBO {
    alignas(16) glm::vec3 position; 
    alignas(16) glm::vec3 velocity;
};

struct SystemUBO
{ 
	float deltaTime;
};


class FVkComputePipeline {
public:
    void Init(); 

    void CreateComputeDescriptorPool();
    void AllocateComputeDescriptorSets();
    void UpdateComputeDescriptorSets();

    void CreateComputePipeline();
    void RecordComputeCommandBuffer(VkCommandBuffer commandBuffer);

    void ComputeTask(std::vector<VkSemaphore>&);

    void CreateComputeCommandBuffers();

	void CreateComputeSyncObjects();


    void UpdateBuffers(float deltaTime);

    
    
    SharedPtr<FVkDevice> deviceRef; 
    SharedPtr<FVkCommandContext> commandContextRef;
	SharedPtr<FVkHeapManager> heapManagerRef;


    SharedPtr<FVkShaderMap> shaderMap;
     

    std::vector<SharedPtr<FVkBuffer>> systemUBOs;
	std::vector<SharedPtr<FVkBuffer>> particleSSBOs;

	std::vector<VkCommandBuffer> computeCommandBuffers;

    VkDescriptorPool computeDescriptorPool;
    std::vector<VkDescriptorSet> computeDescriptorSets;

    VkDescriptorSetLayout computeDescriptorSetLayout;

    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;

    std::vector<VkFence> computeInFlightFences;



    int currentFrame = 0; 
};



//void launchComputeThread(FVkComputePipeline* obj, std::vector<VkSemaphore> renderFinishedSemaphore);
//void terminateCompute();