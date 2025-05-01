


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



    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
    // Initial particle positions on a circle
    std::vector<ParticleSSBO> particles(PARTICLE_COUNT);

    // randome engine is no thread-safe, so...
    for (auto& particle : particles) {
        float r = 0.25f * sqrt(rndDist(rndEngine));
        float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
        float x = r * cos(theta) * HEIGHT / WIDTH;
        float y = r * sin(theta);
        particle.position = glm::vec3(x, y, 0.0f);
        particle.velocity = glm::normalize(glm::vec3(x, y, 0.0f)) * 0.00025f;
    }

    systemInputUBOs.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        systemInputUBOs[i] = CreateShared<FVkBuffer>(deviceRef, 
            FVkBufferDesc{ eUNIFORM_BUFFER, sizeof(SystemInputUBO) });
    }


    particleSSBOs.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        particleSSBOs[i] = CreateShared<FVkBuffer>(deviceRef, 
            FVkBufferDesc{ eSTORAGE_BUFFER, sizeof(ParticleSSBO) * PARTICLE_COUNT});

        heapManagerRef.lock()->updateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            particleSSBOs[i]->buffer, particleSSBOs[i]->memory, sizeof(ParticleSSBO) * PARTICLE_COUNT, particles.data());
    }

	std::cout << "create compute buffers!" << '\n';

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

void FVkComputePipeline::shutdown()
{  

	auto device = deviceRef.lock()->vkDevice;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(device, computeInFlightFences[i], nullptr);
	}

	vkDestroyPipelineLayout(device, computePipelineLayout, nullptr); 
    vkDestroyPipeline(device, computePipeline, nullptr); 
}




void FVkComputePipeline::UpdateInputs(SystemInputUBO ubo) {

	auto device = deviceRef.lock()->vkDevice;

    ubo.deltaTime *= 1000.0f;

    heapManagerRef.lock()->updateBufferMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		systemInputUBOs[currentFrame]->buffer, systemInputUBOs[currentFrame]->memory, sizeof(SystemInputUBO), &ubo);
}

 

void FVkComputePipeline::CreateSyncObjects()
{
    auto device = deviceRef.lock()->vkDevice;
     
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
    auto device = deviceRef.lock()->vkDevice;
    auto computeCommandPool = commandContextRef.lock()->computeCommandPool;

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}



 
void FVkComputePipeline::UpdateDescriptorSets() {

    auto device = deviceRef.lock()->vkDevice;  

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		auto computeDescriptorSet0 = shaderMap->descriptorSets[i][0];
        std::vector<VkWriteDescriptorSet> descriptorWrites{}; 

        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = systemInputUBOs[i]->buffer;
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(SystemInputUBO);

		VkWriteDescriptorSet uboWrites{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        uboWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrites.dstSet = computeDescriptorSet0;
        uboWrites.dstBinding = 0;
        uboWrites.dstArrayElement = 0;
        uboWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrites.descriptorCount = 1;
        uboWrites.pBufferInfo = &uniformBufferInfo;

		descriptorWrites.push_back(uboWrites);


        VkDescriptorBufferInfo storageBufferInfoLastFrame{};
        storageBufferInfoLastFrame.buffer = particleSSBOs[(i - 1) % MAX_FRAMES_IN_FLIGHT]->buffer;
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = sizeof(ParticleSSBO) * PARTICLE_COUNT;


        VkWriteDescriptorSet ssboWrites{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        ssboWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ssboWrites.dstSet = computeDescriptorSet0;
        ssboWrites.dstBinding = 1;
        ssboWrites.dstArrayElement = 0;
        ssboWrites.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        ssboWrites.descriptorCount = 1;
        ssboWrites.pBufferInfo = &storageBufferInfoLastFrame;

        descriptorWrites.push_back(ssboWrites);

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
        storageBufferInfoCurrentFrame.buffer = particleSSBOs[i]->buffer;
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = sizeof(ParticleSSBO) * PARTICLE_COUNT;

        ssboWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ssboWrites.dstSet = computeDescriptorSet0;
        ssboWrites.dstBinding = 2;
        ssboWrites.dstArrayElement = 0;
        ssboWrites.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        ssboWrites.descriptorCount = 1;
        ssboWrites.pBufferInfo = &storageBufferInfoCurrentFrame;

		descriptorWrites.push_back(ssboWrites);

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()) , descriptorWrites.data(), 0, nullptr);
    }
}

void FVkComputePipeline::CreatePipeline() {

	auto device = deviceRef.lock()->vkDevice;
    auto computeShaderModule = shaderMap->shaderModuleMap.at(VK_SHADER_STAGE_COMPUTE_BIT)->shaderModule;

     
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";


    auto descriptorSetLayouts = shaderMap->descriptorSetLayouts;
	auto pushConstantRanges = shaderMap->pushConstantRanges; 

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data(); 

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO }; 
    pipelineInfo.layout = computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    } 
}



void FVkComputePipeline::RecordCommandBuffer(VkCommandBuffer commandBuffer) {

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO }; 

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

	auto size = static_cast<uint32_t>(shaderMap->descriptorSets[currentFrame].size());
	//std::cout << "descriptor set size£º" << size << '\n';
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0,
        size, shaderMap->descriptorSets[currentFrame].data(), 0, nullptr);

    //round up 
	uint32_t dispatchX = ceil_div(PARTICLE_COUNT, static_cast<uint32_t>(256));
    vkCmdDispatch(commandBuffer, dispatchX, 1, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }

	//std::cout << "compute current frame" << currentFrame << '\n';
} 

 
// a persistent thread
void FVkComputePipeline::Compute(std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>& computeFinishedSemaphores)
{   

	auto device = deviceRef.lock()->vkDevice;
    auto computeQueue = commandContextRef.lock()->computeQueue;
     
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
 