#pragma once 


#include <vector>
#include <iostream>
#include <format>

#include <vulkan/vulkan.h>
#include "GVulkanRHI.h"
#include "Shader.h"
#include "Mesh.h"

//
struct FVkPipelineConfig 
{
	 //enable depth
	bool bDepthTest{ true };
	// enable blending
	bool bBlend{ false }; 

};


class FVkGraphicsPipeline
{
public:
	~FVkGraphicsPipeline()
	{ 
		auto& device = deviceRef.lock()->vkDevice;
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		this->pipeline = VK_NULL_HANDLE;
        this->pipelineLayout = VK_NULL_HANDLE;
        this->renderPass = VK_NULL_HANDLE;

	}
	explicit FVkGraphicsPipeline(WeakPtr<FVkDevice> device, WeakPtr<FVkShaderMap> shaderMap) : deviceRef(device), shaderMap(shaderMap)
	{ 
		this->renderPass = createRenderPass();
		
		this->pipelineLayout = createPipelineLayout();
		//after render pass
		this->pipeline = createGraphicsPipeline();
	}

	//delete copy and mvoe
	FVkGraphicsPipeline(const FVkGraphicsPipeline&) = delete;
	FVkGraphicsPipeline& operator=(const FVkGraphicsPipeline&) = delete;

	FVkGraphicsPipeline(FVkGraphicsPipeline&&) = delete;
	FVkGraphicsPipeline& operator=(FVkGraphicsPipeline&&) = delete;


	 

	[[nodiscard]] VkRenderPass createRenderPass() const;
	[[nodiscard]] VkPipeline createGraphicsPipeline() const;
	[[nodiscard]] VkPipelineLayout createPipelineLayout() const;

	VkRenderPass renderPass{ VK_NULL_HANDLE }; 
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE }; //for the bind desets
	VkPipeline pipeline	{ VK_NULL_HANDLE };

private:
	const WeakPtr<FVkShaderMap> shaderMap;
	const WeakPtr<FVkDevice> deviceRef;
};
