#pragma once


#include <vulkan/vulkan.h> 

#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"
 
#include "ComputeThread.h"


class FVkSceneRenderer
{
public:

	void Init();
	void Shutdown();
	void Update();
	void Render();
	void recordRenderCommand(VkCommandBuffer graphicsCmdBuffer, uint32_t imageIndex);


	void CreateCommandBuffers();

	void CreateSyncObjects();
	void CreateDescriptorPool();
	void AllocateDescriptorSets();
	void UpdateDescriptorSets();


	void CreateRenderPass();
	void CreateSwapChainFramebuffers();
	void CreateGraphicsPipeline();


	//universal, global context
	//===========================
	SharedPtr<FVkWindow> window; 
	SharedPtr<FVkDevice> deviceRef; 
	SharedPtr<FVkCommandContext> commandContextRef; 

	//

	 SharedPtr<FVkSwapChain> swapChainRef;

	SharedPtr<FVkTexture> FBDepthTexture;
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;  //hardcode for now

	//depend on application , pipeline-unaware
	//===========================
	//scene ,assets
	SharedPtr<UStaticMesh> mesh; 

	SharedPtr<FVkShaderMap> shaderMap;

	SharedPtr<FVkTexture> texture; 


	//depend on renderer design
	//=========================== 
	 


	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE; //for the bind desets
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;


	//consider set 0 for now;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets;


	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores; 
	std::vector<VkFence> renderInFlightFences;

	
	 
	uint32_t currentFrame = 0; 
	double deltaTime;

	std::vector<VkCommandBuffer> commandBuffers;

	//new:
	FVkComputePipeline* computePipeline; 
	std::vector<VkSemaphore> computeFinishedSemaphores;

};