#pragma once


#include <vulkan/vulkan.h> 

#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"
 
#include "ComputePipeline.h"


class FVkGraphicsPipeline
{
public:

	void init();
	void shutdown();
	void update();
	void render();
	void recordRenderCommand(VkCommandBuffer graphicsCmdBuffer, uint32_t imageIndex);


	void createCommandBuffers(); 
	void createSyncObjects(); 

	virtual void createSceneResources();
	virtual void updateDescriptorSets(); 

	void createRenderPass();
	void createSwapChainFramebuffers();
	void createGraphicsPipeline();


	//system
	//===========================
	SharedPtr<FVkWindow> window; 
	SharedPtr<FVkDevice> deviceRef; 
	SharedPtr<FVkCommandContext> commandContextRef; 
	SharedPtr<FVkHeapManager> heapManagerRef;

	// 
	SharedPtr<FVkSwapChain> swapChainRef;

	SharedPtr<FVkTexture> FBDepthTexture;
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;  //hardcode for now

	//depend on application , pipeline-unaware
	//===========================
	//scene  
	SharedPtr<UStaticMesh> mesh;

	SharedPtr<FVkShaderMap> shaderMap;

	std::array<SharedPtr<FVkTexture>, MAX_FRAMES_IN_FLIGHT> textures;


	//depend on shaders, and renderer design
	//===========================  

	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	 
	//
	//std::array< std::vector<VkDescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorSets;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE; //for the bind desets
	VkPipeline pipeline = VK_NULL_HANDLE;


	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> graphicsCommandBuffers;

	 
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> renderInFlightFences;

	
	uint32_t currentFrame = 0;
	double deltaTime;
	double elapsedTime;

	int frameCounter = 0;

	//new:
	FVkComputePipeline* computePipeline; 
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeFinishedSemaphores;

};