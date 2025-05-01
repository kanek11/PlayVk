#include "Renderer.h"



#include <thread>
#include <atomic> 






void FVkGraphicsPipeline::shutdown()
{
}


void FVkGraphicsPipeline::update()
{
	auto window = this->window->pGLFWWindow;
	auto device = deviceRef->vkDevice;


	std::cout << "begin loop on main thread id: " << std::this_thread::get_id() << std::endl; 

	double startTime = glfwGetTime();
	double lastFrameTime = startTime;
	double currentTime = startTime;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		render();

		lastFrameTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		elapsedTime = currentTime - startTime;

		frameCounter++;

		//std::cout << "elapsed time" << elapsedTime << std::endl;
	}

	vkDeviceWaitIdle(device);  //wait for device to finish async operations 
}

void InsertImageBarrier(VkCommandBuffer commandBuffer, VkImage image,
	FVkImageStageConfig& srcStage, FVkImageStageConfig& dstStage,
	uint32_t mipLevels = 1);

void FVkGraphicsPipeline::render()
{
	auto device = deviceRef->vkDevice;
	auto swapChain = swapChainRef->vkSwapChain;
	auto graphicsQueue = commandContextRef->graphicsQueue;
	auto presentQueue = commandContextRef->presentQueue;  

	FVkImageStageConfig initStage{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 0 };
	FVkImageStageConfig computeStage{ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT };
	FVkImageStageConfig renderStage{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT };



	//new: 
	SystemInputUBO ubo{
	.iResolution = glm::vec2(WIDTH, HEIGHT),
	.iTime = static_cast<float>(elapsedTime),
	.iTimeDelta = static_cast<float>(deltaTime),
	.iFrame = frameCounter,
	};

	computePipeline->UpdateInputs(ubo);
	//std::cout << "deltatime" << deltaTime << std::endl;
    computePipeline->Compute(computeFinishedSemaphores); 


	auto commandBuffer2 = commandContextRef->beginGraphicsTransientCmdBuffer();
	InsertImageBarrier(commandBuffer2, textures[currentFrame]->image, computeStage, renderStage);
	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer2);


	vkWaitForFences(device, 1, &renderInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	//any application update
	{
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	/*	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}*/


	vkResetCommandBuffer(graphicsCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

	recordRenderCommand(graphicsCommandBuffers[currentFrame], imageIndex);


	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	 
	std::vector< VkSemaphore> waitSemaphores = { 
		imageAvailableSemaphores[currentFrame] ,
		computeFinishedSemaphores[currentFrame]
	};
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();



	//CPU-GPU, and GPU-GPU sync can be different,
	//which stage will be blocked for waiting semaphore. 
	//for presentation, the geometry can fire away, but write to color attachment should wait.
	VkPipelineStageFlags waitDstStages[] = { 
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitDstStages;  


	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &graphicsCommandBuffers[currentFrame];  //not the array;

	//new:
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, &renderInFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderInFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	auto commandBuffer = commandContextRef->beginGraphicsTransientCmdBuffer();
	InsertImageBarrier(commandBuffer, textures[currentFrame]->image, renderStage, computeStage);
	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer);


	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	//
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}



void FVkGraphicsPipeline::recordRenderCommand(VkCommandBuffer graphicsCmdBuffer, uint32_t imageIndex)
{

	auto swapChainExtent = swapChainRef->swapChainExtent;

	//std::cout << "resolution£º " << swapChainExtent.width << " " << swapChainExtent.height << std::endl;

	//todo: avoid copy? 

	VkBuffer vertexBuffer = mesh->staticMeshResource->vertexBuffer->buffer;
	VkBuffer indexBuffer = mesh->staticMeshResource->indexBuffer->buffer; 
	if (vertexBuffer == VK_NULL_HANDLE || indexBuffer == VK_NULL_HANDLE)
	{
		throw std::runtime_error("empty handles?");
	}



	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	if (vkBeginCommandBuffer(graphicsCmdBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.1f, 0.1f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 }; 
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();


	vkCmdBeginRenderPass(graphicsCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(graphicsCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(graphicsCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(graphicsCmdBuffer, 0, 1, &scissor);

	//todo: less hardcoded? 
	{ 
		auto size = static_cast<uint32_t>(shaderMap->descriptorSets[currentFrame].size());
		vkCmdBindDescriptorSets(graphicsCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pipelineLayout, 0, size , shaderMap->descriptorSets[currentFrame].data(), 0, nullptr);
	 
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(graphicsCmdBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(graphicsCmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	}
	 
	vkCmdDrawIndexed(graphicsCmdBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
	//vkCmdDrawIndexed(graphicsCmdBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
	//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0); 

	vkCmdEndRenderPass(graphicsCmdBuffer);

	if (vkEndCommandBuffer(graphicsCmdBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

}







//only graphics for now
void FVkGraphicsPipeline::createCommandBuffers()
{
	auto device = deviceRef->vkDevice;
	auto graphicsCommandPool = commandContextRef->graphicsCommandPool; 

	VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

}



void FVkGraphicsPipeline::createSyncObjects()
{
	auto device = deviceRef->vkDevice; 


	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &renderInFlightFences[i]) != VK_SUCCESS   )
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

 

void FVkGraphicsPipeline::updateDescriptorSets()
{
	auto device = deviceRef->vkDevice; 

	auto imageBindings = shaderMap->parameterMap->samplerMap; 
	const char* name = "texSampler";
	auto imageBinding = imageBindings.at(name);
	std::cout << "reflect: " << name << " bound to " << imageBinding.binding << std::endl; 
 

	auto descriptorSets = shaderMap->descriptorSets;
   
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {  
			 
		auto descriptorSets0 = descriptorSets[i][0];
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textures[i]->imageView;
		imageInfo.sampler = textures[i]->sampler;

		VkWriteDescriptorSet samplerWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		samplerWrite.dstSet = descriptorSets0;
		samplerWrite.dstBinding = imageBinding.binding;
		samplerWrite.dstArrayElement = 0;
		samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerWrite.descriptorCount = 1;
		samplerWrite.pImageInfo = &imageInfo;

		descriptorWrites.push_back(samplerWrite);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);  
	 
	} 

}






void FVkGraphicsPipeline::createRenderPass()
{
	auto device = deviceRef->vkDevice;

	auto format = swapChainRef->swapChainImageFormat;
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkSubpassDescription> subpasses;

	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  //multisampling
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		attachments.push_back(colorAttachment);


		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
		attachments.push_back(depthAttachment);
	
	}




	{
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRefs.push_back(colorAttachmentRef);

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		subpasses.push_back(subpass);
	}



	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}



void FVkGraphicsPipeline::createSwapChainFramebuffers()
{
	auto device = deviceRef->vkDevice;
	auto extent = swapChainRef->swapChainExtent;
	auto swapChainImageViews = swapChainRef->swapChainImageViews; 
	auto depthImageView = FBDepthTexture->imageView;

	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {

		std::array<VkImageView, 2> attachments = {
	         swapChainImageViews[i],
	         depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

	}

	std::cout << "\tapp: swapchain FB created!" << std::endl;

}


void FVkGraphicsPipeline::createGraphicsPipeline()
{

	auto device = deviceRef->vkDevice; 

	//shader module;
	//todo : automize this

	auto shaderModuels = shaderMap->shaderModuleMap;

	VkShaderModule vertShaderModule = shaderModuels.at(VK_SHADER_STAGE_VERTEX_BIT)->shaderModule;
	VkShaderModule fragShaderModule = shaderModuels.at(VK_SHADER_STAGE_FRAGMENT_BIT)->shaderModule;

	if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE)
	{
		throw std::runtime_error("empty handles?");
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };



	//shader mapped data 
	auto pushConstantRanges = shaderMap->pushConstantRanges;
	auto descriptorSetLayouts = shaderMap->descriptorSetLayouts;  
	//std::cout << "bind pipelinelayout size:" << descriptorSetLayouts.size() << std::endl;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();


	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}



	//geometry
	//todo : how different objects handle this while reusing the pipeline?

	auto staticMeshResource = mesh->staticMeshResource;
	//new: 
	auto bindingDescriptions = staticMeshResource->bindingDescriptions;
	auto attributeDescriptions = staticMeshResource->attributeDescriptions;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();


	VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	//raster

	VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // LINE; POINT:
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;



	VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencil.depthTestEnable = VK_FALSE; // VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;



	VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	//dynamic
	VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();




	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = 2;  //adjust count here;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	std::cout << "\tapp: graphics pipeline created!" << std::endl;


}






void FVkGraphicsPipeline::init()
{
	//system setup
	this->window = CreateShared<FVkWindow>();
	Global::vulkanRHI = new GVulkanRHI(window);

	this->deviceRef = Global::vulkanRHI->deviceRef;

	this->commandContextRef = Global::vulkanRHI->commandContextRef;

	this->heapManagerRef = Global::vulkanRHI->heapManagerRef;

	this->swapChainRef = Global::vulkanRHI->swapChainRef;

	 
	 
	//pipeline common
	createCommandBuffers();

	createSyncObjects();  

	//app specific:  

	//framebuffers
	this->FBDepthTexture = CreateShared<FVkTexture>(deviceRef);
	this->FBDepthTexture->createFBDepthTexture(swapChainRef->swapChainExtent.width, 
		swapChainRef->swapChainExtent.height,
		depthFormat);

	createRenderPass();
	createSwapChainFramebuffers(); 
	 
	 
	this->computePipeline = new FVkComputePipeline{};
	computePipeline->init();

	createSceneResources(); 

	//the pipeline also dependent on descriptors, mesh ..
	createGraphicsPipeline();

	updateDescriptorSets(); //dependent on buffer from compute

}



void FVkGraphicsPipeline::createSceneResources()
{
	this->mesh = CreateShared<UStaticMesh>();

	//define a rectangle
	std::vector<glm::vec3> pos = {
		glm::vec3 {-1.0f,  1.0f, 0.0f, },
		glm::vec3 {-1.0f, -1.0f, 0.0f, },
		glm::vec3 { 1.0f,  1.0f, 0.0f, },
		glm::vec3 { 1.0f, -1.0f, 0.0f, },
	};

	std::vector<uint16_t> indices = {
		 0, 1, 2, 2, 1, 3,
	};

	std::vector<glm::vec2> UV =
	{
	   glm::vec2 { 0.0f, 1.0f },
	   glm::vec2 { 0.0f, 0.0f },
	   glm::vec2 { 1.0f, 1.0f },
	   glm::vec2 { 1.0f, 0.0f },
	};


	mesh->positions = pos;
	mesh->UVs = UV;
	mesh->indices = indices;
	mesh->CreateRHIResource();


	auto testShaderMapBuilder = FVkShaderMapBuilder(deviceRef);
	this->shaderMap = testShaderMapBuilder
		.SetVertexShader("shaders/bin/testVert.glsl.spv")
		.SetFragmentShader("shaders/bin/testFrag.glsl.spv")
		.Build();

	shaderMap->reflectShaderParameters(); 


	this->textures = computePipeline->textures;

}