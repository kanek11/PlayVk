


#include "ComputeThread.h"


#include <thread>
#include <atomic> 

#include <random>

//include openmp
#include <omp.h>


//std::thread computeThread;
//std::atomic<bool> terminateComputeThread(false);

void FVkComputePipeline::Init() {
      
    this->deviceRef = Global::vulkanRHI->deviceRef;
    this->commandContextRef = Global::vulkanRHI->commandContextRef; 

    this->heapManagerRef = Global::vulkanRHI->heapManagerRef;


    auto computeShader = CreateShared<FVkShaderModule>(deviceRef, VK_SHADER_STAGE_COMPUTE_BIT,
        "shaders/bin/testCompute.glsl.spv");

	this->shaderMap = CreateShared<FVkShaderMap>(deviceRef);
	shaderMap->AddShaderModule(VK_SHADER_STAGE_COMPUTE_BIT, computeShader);

    shaderMap->ReflectShaderParameters(); 

	this->computeDescriptorSetLayout = shaderMap->descriptorSetLayout;



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

    systemUBOs.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        systemUBOs[i] = CreateShared<FVkBuffer>(deviceRef, UNIFORM_BUFFER, sizeof(SystemUBO)); 
	}
     

    particleSSBOs.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		particleSSBOs[i] = CreateShared<FVkBuffer>(deviceRef, STORAGE_BUFFER, sizeof(ParticleSSBO) * PARTICLE_COUNT);
        heapManagerRef->UpdateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            particleSSBOs[i]->buffer, particleSSBOs[i]->memory, sizeof(ParticleSSBO) * PARTICLE_COUNT, particles.data());
	}





    CreateComputeSyncObjects();

	CreateComputeCommandBuffers();

	CreateComputeDescriptorPool();
	AllocateComputeDescriptorSets();
	UpdateComputeDescriptorSets();

	CreateComputePipeline(); 

}




void FVkComputePipeline::UpdateBuffers(float deltaTime) {

	auto device = deviceRef->vkDevice;

	SystemUBO ubo{};
	ubo.deltaTime = deltaTime;

    heapManagerRef->UpdateBufferMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		systemUBOs[currentFrame]->buffer, systemUBOs[currentFrame]->memory, sizeof(SystemUBO), &ubo);
}






void FVkComputePipeline::CreateComputeSyncObjects()
{
    auto device = deviceRef->vkDevice;
     
    computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
     
    //VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; 

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(device, &fenceInfo, nullptr,  &computeInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void FVkComputePipeline::CreateComputeCommandBuffers()
{
    auto device = deviceRef->vkDevice;
    auto computeCommandPool = commandContextRef->computeCommandPool;

    computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(computeCommandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}

 
void FVkComputePipeline::CreateComputeDescriptorPool() {

    auto device = deviceRef->vkDevice;

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 10;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 10;

    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 10;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &computeDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}


void FVkComputePipeline::AllocateComputeDescriptorSets() {

    auto device = deviceRef->vkDevice;  

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = computeDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    computeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, computeDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

}

void FVkComputePipeline::UpdateComputeDescriptorSets() {

    auto device = deviceRef->vkDevice; 

	computeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT); 

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = systemUBOs[i]->buffer;
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(SystemUBO);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = computeDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

        VkDescriptorBufferInfo storageBufferInfoLastFrame{};
        storageBufferInfoLastFrame.buffer = particleSSBOs[(i - 1) % MAX_FRAMES_IN_FLIGHT]->buffer;
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = sizeof(ParticleSSBO) * PARTICLE_COUNT;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = computeDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
        storageBufferInfoCurrentFrame.buffer = particleSSBOs[i]->buffer;
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = sizeof(ParticleSSBO) * PARTICLE_COUNT;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = computeDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

        vkUpdateDescriptorSets(device, 3, descriptorWrites.data(), 0, nullptr);
    }
}

void FVkComputePipeline::CreateComputePipeline() {

	auto device = deviceRef->vkDevice;
    auto computeShaderModule = shaderMap->shaderModuleMap.at(VK_SHADER_STAGE_COMPUTE_BIT)->shaderModule;

     
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

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



void FVkComputePipeline::RecordComputeCommandBuffer(VkCommandBuffer commandBuffer) {

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO }; 

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSets[currentFrame], 0, nullptr);

    //round up 
	uint32_t dispatchX = static_cast<uint32_t>(std::ceil((float)PARTICLE_COUNT / (float)256 ));
    vkCmdDispatch(commandBuffer, dispatchX, 1, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }

	//std::cout << "compute current frame" << currentFrame << std::endl;
} 

 
// a persistent thread
void FVkComputePipeline::ComputeTask(std::vector<VkSemaphore>& computeFinishedSemaphores)
{  
   //std::cout << "compute thread launched" << std::endl;
	//std::cout << "on thread id:" << std::this_thread::get_id() << std::endl;


	auto device = deviceRef->vkDevice;
    auto computeQueue = commandContextRef->computeQueue;

    //while (!terminateComputeThread) {  
    vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

 
    vkResetCommandBuffer(computeCommandBuffers[currentFrame], 0);
    RecordComputeCommandBuffer(computeCommandBuffers[currentFrame]); 
    
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

 

//void launchComputeThread(FVkComputePipeline* obj, std::vector<VkSemaphore> renderFinishedSemaphore) {
//    computeThread = std::thread(&FVkComputePipeline::ComputeTask, obj, renderFinishedSemaphore); 
//}
//
//void terminateCompute() {
//    terminateComputeThread = true;
//    computeThread.join();  // 等待线程安全退出
//}
//
