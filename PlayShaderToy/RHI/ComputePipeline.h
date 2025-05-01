#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"
 

struct alignas(16) SystemInputUBO
{
    alignas(8) glm::vec2 iResolution;
    float iTime;
    float iTimeDelta;
    int iFrame;
    alignas(16) glm::vec4 iMouse; 

    glm::vec2 iChannelResolution[4];
};



class FVkComputePipeline {
public:
    void init(); 

    virtual void CreateSceneResources();
    virtual void UpdateDescriptorSets();

    void CreatePipeline();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer); 
    void Compute(std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>&); 
    void UpdateInputs(SystemInputUBO deltaTime); 

    void CreateCommandBuffers(); 
	void CreateSyncObjects();
     
    
    
    SharedPtr<FVkDevice> deviceRef; 
    SharedPtr<FVkCommandContext> commandContextRef;
	SharedPtr<FVkHeapManager> heapManagerRef;


    SharedPtr<FVkShaderMap> shaderMap;

    std::array< SharedPtr<FVkTexture>, MAX_FRAMES_IN_FLIGHT> textures;
	std::vector< SharedPtr<FVkTexture>> iChannelTextures;

    std::array<SharedPtr<FVkBuffer>, MAX_FRAMES_IN_FLIGHT> systemInputUBOs; 



    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> computeCommandBuffers;
      
    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeInFlightFences; 


    int currentFrame = 0; 
};
 