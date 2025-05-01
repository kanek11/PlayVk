


#include "ComputePipeline.h"


#include <thread>
#include <atomic> 

#include <random>   



void FVkComputePipeline::CreateSceneResources()
{
    auto computeShader = CreateShared<FVkShaderModule>(deviceRef, VK_SHADER_STAGE_COMPUTE_BIT,
        "shaders/bin/testCompute.glsl.spv");

    this->shaderMap = CreateShared<FVkShaderMap>(deviceRef);
    shaderMap->addShaderModule(VK_SHADER_STAGE_COMPUTE_BIT, computeShader);

    shaderMap->reflectShaderParameters(); 

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        textures[i] = CreateShared<FVkTexture>(deviceRef);
        textures[i]->createStorageImage(WIDTH, HEIGHT);
    }

    ImageDesc imageMetaInfo{};
    void* data = nullptr;
    //LoadImage("images/noise.png", imageMetaInfo, data);
    LoadImage("images/pebble.jpg", imageMetaInfo, data);

    auto iChannel0 = CreateShared<FVkTexture>(deviceRef);
    iChannel0->createTexture(imageMetaInfo, data); 
    iChannelTextures.push_back(iChannel0);

    //
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        systemInputUBOs[i] = CreateShared<FVkBuffer>(deviceRef, eUNIFORM_BUFFER, sizeof(SystemInputUBO));
    }

}






void FVkComputePipeline::init() {
      
    this->deviceRef = Global::vulkanRHI->deviceRef;
    this->commandContextRef = Global::vulkanRHI->commandContextRef; 

    this->heapManagerRef = Global::vulkanRHI->heapManagerRef; 
     

    CreateSyncObjects();

    CreateCommandBuffers(); 

    CreateSceneResources(); 


	CreatePipeline(); 
     
    UpdateDescriptorSets();

}




void FVkComputePipeline::UpdateInputs(SystemInputUBO ubo) {

    auto device = deviceRef->vkDevice;

    //ubo.iTimeDelta *= 1000.0f;
    for (size_t i = 0; i < iChannelTextures.size(); i++) {
		auto extent = iChannelTextures[i]->imageConfig.extent;
		ubo.iChannelResolution[i] = glm::vec2(extent.width, extent.height);
    }
    

    heapManagerRef->updateBufferMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		systemInputUBOs[currentFrame]->buffer, systemInputUBOs[currentFrame]->memory, sizeof(SystemInputUBO), &ubo);
}

 

void FVkComputePipeline::CreateSyncObjects()
{
    auto device = deviceRef->vkDevice; 
     
    //VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; 

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(device, &fenceInfo, nullptr,  &computeInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void FVkComputePipeline::CreateCommandBuffers()
{
    auto device = deviceRef->vkDevice;
    auto computeCommandPool = commandContextRef->computeCommandPool; 

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}



 
void FVkComputePipeline::UpdateDescriptorSets() {

    auto device = deviceRef->vkDevice;  

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		auto set0 = shaderMap->descriptorSets[i][0];  

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = systemInputUBOs[i]->buffer;
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(SystemInputUBO);

        VkWriteDescriptorSet uboWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = set0;
        uboWrite.dstBinding = 0;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &uniformBufferInfo;

        descriptorWrites.push_back(uboWrite);

        uint32_t lastFrame = (i - 1) % MAX_FRAMES_IN_FLIGHT;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.imageView = textures[lastFrame]->imageView;
        //swap?

        VkWriteDescriptorSet imageWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        imageWrite.dstSet = set0;
        imageWrite.dstBinding = 1;
        imageWrite.dstArrayElement = 0;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        imageWrite.descriptorCount = 1;
        imageWrite.pImageInfo = &imageInfo;

        descriptorWrites.push_back(imageWrite);



        auto set1 = shaderMap->descriptorSets[i][1];
		for (size_t i = 0; i < iChannelTextures.size(); i++)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = iChannelTextures[i]->imageView;
			imageInfo.sampler = iChannelTextures[i]->sampler;

			VkWriteDescriptorSet samplerWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			samplerWrite.dstSet = set1;
			samplerWrite.dstBinding = i;
			samplerWrite.dstArrayElement = 0;
			samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerWrite.descriptorCount = 1;
			samplerWrite.pImageInfo = &imageInfo;

			descriptorWrites.push_back(samplerWrite);
		} 

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
   
		std::cout << "updated resource num:" << descriptorWrites.size() << std::endl;
    
    }
}

void FVkComputePipeline::CreatePipeline() {

	auto device = deviceRef->vkDevice;
    auto computeShaderModule = shaderMap->shaderModuleMap.at(VK_SHADER_STAGE_COMPUTE_BIT)->shaderModule;

     
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";



	auto descriptorSetLayouts = shaderMap->descriptorSetLayouts;
    //std::cout << "bind pipelinelayout size:" << descriptorSetLayouts.size() << std::endl;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO }; 
    pipelineInfo.layout = computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}



void FVkComputePipeline::RecordCommandBuffer(VkCommandBuffer commandBuffer) {

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO }; 

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

	auto size = static_cast<uint32_t>(shaderMap->descriptorSets[currentFrame].size());
	//std::cout << "descriptor set size£º" << size << std::endl;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0,
        size, shaderMap->descriptorSets[currentFrame].data(), 0, nullptr);

    //round up 
    uint32_t dispatchX = static_cast<uint32_t>(std::ceil((float)WIDTH / (float)16));
    uint32_t dispatchY = static_cast<uint32_t>(std::ceil((float)HEIGHT / (float)16));
    vkCmdDispatch(commandBuffer, dispatchX, dispatchY, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }

	//std::cout << "compute current frame" << currentFrame << std::endl;
} 

 
// a persistent thread
void FVkComputePipeline::Compute(std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>& computeFinishedSemaphores)
{   

	auto device = deviceRef->vkDevice;
    auto computeQueue = commandContextRef->computeQueue;
     
    vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX); 
 
    vkResetCommandBuffer(computeCommandBuffers[currentFrame], 0);
    RecordCommandBuffer(computeCommandBuffers[currentFrame]); 
    
    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };  

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

    vkResetFences(device, 1, &computeInFlightFences[currentFrame]);

    if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    } 
   
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
}
 