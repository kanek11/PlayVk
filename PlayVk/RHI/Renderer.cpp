#include "Renderer.h"
 
#include "LoaderModule/LoaderModule.h"

#include <thread>
#include <atomic>  

#include "Timer.h"

FVkRenderer::FVkRenderer()
{
	//system setup
	this->window = CreateShared<FVkWindow>();
	auto rhi = new GVulkanRHI(window);
	Global::vulkanRHI = rhi;


	this->deviceRef = rhi->deviceRef;
	this->commandContextRef = rhi->commandContextRef;
	this->heapManagerRef = rhi->heapManagerRef;
	this->swapChainRef = rhi->swapChainRef; 
}

FVkRenderer::~FVkRenderer()
{
	delete Global::vulkanRHI;
}


void  FVkRenderer::update() noexcept
{
	auto& window = this->window->pGLFWWindow;
	auto& device = deviceRef.lock()->vkDevice;


	std::cout << "begin loop on main thread id: " << std::this_thread::get_id() << '\n'; 
	std::cout << "the scene has obj num:" << meshProxies.size() << '\n';

	double currentTime = glfwGetTime();
	double lastFrameTime = currentTime;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents(); 

		lastFrameTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		 
		render(); 
	}

	vkDeviceWaitIdle(device);  //wait for device to finish async operations 
}



void  FVkRenderer::render() noexcept
{
	auto& device = deviceRef.lock()->vkDevice;
	auto& swapChain = swapChainRef.lock()->vkSwapChain;
	auto& graphicsQueue = commandContextRef.lock()->graphicsQueue;
	auto& presentQueue = commandContextRef.lock()->presentQueue;  
	 

	vkWaitForFences(device, 1, &renderInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	//any application update
	{
	}

	uint32_t imageIndex{};
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	//std::cout << "acquire image index: " << imageIndex << '\n';
	/*	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}*/


	vkResetCommandBuffer(graphicsCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	 
    recordRenderCommand(graphicsCommandBuffers[currentFrame], imageIndex); 


	VkSubmitInfo submitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	 
	std::vector< VkSemaphore> waitSemaphores = { 
		imageAvailableSemaphores[currentFrame] ,
		//computeFinishedSemaphores[currentFrame]
	};
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
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
		std::cerr << "failed to submit command!" << '\n';
		return;
	}


	VkPresentInfoKHR presentInfo{ .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

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



void FVkRenderer::recordRenderCommand(VkCommandBuffer cmdBuffer, uint32_t imageIndex) noexcept
{ 
	auto swapChainExtent = *(swapChainRef.lock()->swapChainExtent);

	auto pipelineLayout = graphicsPipeline->pipelineLayout;
	auto renderPass = graphicsPipeline->renderPass;
	auto pipeline = graphicsPipeline->pipeline; 


	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
		std::cerr << "failed to begin recording command buffer!" << '\n';
		return;
	}

	VkRenderPassBeginInfo renderPassBeginInfo{ .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex]; //similar to binding framebuffer in OpenGL
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 }; 
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();


	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	 

	VkViewport viewport{};
	viewport.x = 0.0f; 
	viewport.width = (float)swapChainExtent.width;
	viewport.y = 0.0f;
	viewport.height = (float)swapChainExtent.height; 

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	//new:
	//uint32_t debugCounter{ 0 };
	for (const auto& proxy : meshProxies)
	{  
		//std::cout << "mesh index" << debugCounter++ << '\n';
		//VkBuffer vertexBuffer = mesh->staticMeshResource->vertexBuffer->buffer;
		//VkBuffer indexBuffer = mesh->staticMeshResource->indexBuffer->buffer;   
		VkBuffer vertexBuffer = proxy.vertexBuffer;
		VkBuffer indexBuffer = proxy.indexBuffer; 

		auto& bindings = proxy.descriptorSets[currentFrame]; 
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,  pipelineLayout, 0,
			static_cast<uint32_t>(bindings.size()), bindings.data(), 0, nullptr); 

		if (proxy.objectConstants.has_value())
		vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 
			static_cast <uint32_t>(sizeof(FObjectConstants)), &proxy.objectConstants); 

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(cmdBuffer, indexBuffer, 0, proxy.indexType);
	
		//auto indexCount = static_cast<uint32_t>(mesh->indices.size());
		vkCmdDrawIndexed(cmdBuffer, proxy.indexCount,  proxy.instanceCount , 0, 0, 0); 
	}
	  

	vkCmdEndRenderPass(cmdBuffer);

	if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
		std::cerr << "failed to end recording command buffer!" << '\n';
		return;
	}

}
 


//todo: only graphics for now ; maybe delegate to context;
void  FVkRenderer::createCommandBuffers()
{
	auto& device = deviceRef.lock()->vkDevice;
	auto& graphicsCommandPool = commandContextRef.lock()->graphicsCommandPool; 

	VkCommandBufferAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

}



void  FVkRenderer::createSyncObjects()
{
	auto& device = deviceRef.lock()->vkDevice;


	VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			//vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &renderInFlightFences[i]) != VK_SUCCESS   )
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

 
 

void FVkRenderer::createSCFramebuffers()
{
	auto& device = deviceRef.lock()->vkDevice;
	auto& extent = swapChainRef.lock()->swapChainExtent.value();
	auto& swapChainImageViews = swapChainRef.lock()->swapChainImageViews;
	auto& depthImageView = FBDepthTexture->imageView;
	auto& renderPass = graphicsPipeline->renderPass;

	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {

		std::array<VkImageView, 2> attachments = {
	         swapChainImageViews[i],
	         depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{ .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
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

	std::cout << "\tapp: swapchain FB created!" << '\n';

}









void  FVkRenderer::init()
{ 
	 
	//pipeline common
	createCommandBuffers();

	createSyncObjects();  

	//app specific:   
	createSceneResources(); 
	 
	initBindings(); 

	if (!shaderManager->validateBindings()) {
		throw std::runtime_error("failed to validate descriptor sets!");
	}

	//after scene, depend on shader reflection;
	this->graphicsPipeline = CreateShared<FVkGraphicsPipeline>(deviceRef, shaderManager);  

	//after renderpass
	auto swapChainExtent = swapChainRef.lock()->swapChainExtent.value();
	auto depthFormat = swapChainRef.lock()->depthFormat.value();

	this->FBDepthTexture = CreateShared<FVkTexture>(deviceRef);
	this->FBDepthTexture->createFBDepthTexture(swapChainExtent.width, swapChainExtent.height, depthFormat);

	createSCFramebuffers();  


}


void  FVkRenderer::shutdown()
{
	const auto& device = deviceRef.lock()->vkDevice;

	//computePipeline.reset();
	graphicsPipeline.reset();

	FBDepthTexture.reset();

	for (auto& framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto& mesh : meshes) {
		//std::cout << " " << mesh.use_count() << '\n';
		mesh.reset();
	}
	for (auto& tex : textures) {
		tex.reset();
	} 
	debugTexture.reset();

	//new£»
	cameraUBO.reset();

	shaderManager.reset(); 


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		//vkDestroySemaphore(device, computeFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, renderInFlightFences[i], nullptr);
	}
}


//todo: more granular
void  FVkRenderer::initBindings()
{  
	 
	for (auto& proxy : meshProxies) { 
		 
		for (uint32_t iFrame = 0; iFrame < MAX_FRAMES_IN_FLIGHT; iFrame++) {
			 
			//new:
			FVkBufferRTInfo cameraUBOInfo { cameraUBO->buffer, 0, sizeof(FCameraUBO) };
			shaderManager->bindUniformBuffer("CameraUBO", cameraUBOInfo);

			//auto tex = debugTexture;
			//auto tex = textures[0];

			auto tex = proxy.texture.lock();
			if (!tex) {
				 tex = debugTexture;  //fallback
				 std::cout << "fallback to debug texture\n";
			}
			
			FVkTextureRTInfo textureInfo = { tex->image, tex->imageView, tex->sampler };
			shaderManager->bindTexture("Sampler0", textureInfo);


			shaderManager->allocateDescriptorSets(proxy.descriptorSets[iFrame]);
			shaderManager->updateBindings(proxy.descriptorSets[iFrame]);
		}

	} 

}



void  FVkRenderer::createSceneResources()
{
	//scene  
	FCameraUBO cameraUBOData{
		.view = glm::lookAt(this->cameraPos, glm::vec3(0.0f), this->up),  //vulkan viewport is flipped;
		//.proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f), 
		.proj = glm::perspectiveRH_ZO(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f),
	};

	this->cameraUBO = CreateShared<FVkBuffer>(deviceRef, 
		FVkBufferDesc{ eUNIFORM_BUFFER, sizeof(FCameraUBO) });  

	heapManagerRef.lock()->updateBufferMemory(
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		cameraUBO->buffer, cameraUBO->memory, sizeof(FCameraUBO), &cameraUBOData);



	auto testShaderMapBuilder = FVkShaderMapBuilder(deviceRef);
	this->shaderManager = testShaderMapBuilder
		//.SetVertexShader("shaders/bin/particleVert.glsl.spv")
		//.SetVertexShader("shaders/bin/staticVert.glsl.spv")
		//.SetFragmentShader("shaders/bin/sampleFrag.glsl.spv")
		 .SetVertexShader("shaders/bin/staticModel_VS.spv")
		 .SetFragmentShader("shaders/bin/staticModel_PS.spv")
		.Build();

	shaderManager->reflectShaderParameters(); 

	//new:
 
	using namespace Loader; 
 //   if(true)
	//{
	//	this->modelDir = "D:/CG_resources/dae/vampire";
	//	this->modelName = "dancing_vampire.dae"; 
	//	this->scale = 0.01f;
	//}
	//if(false)
	//{
	//	this->modelDir = "D:/CG_resources/my";
	//	this->modelName = "A2.dae";  
	//	this->scale = 1.0f;
	//	 
	//}
	if (true)
	{
		this->modelDir = "D:/CG_resources/classic/cube";
		this->modelName = "cube.obj"; 
		this->scale = 0.5f;
	}

 

	Timer<> LoadTimer;

	auto scene = loadSceneData((modelDir / modelName).string(), 
		{ .scale = this->scale }); 

	//manually destroy timer:
    cout << "mesh loading:" << '\n';
    LoadTimer.stop();


	//auto sceneOpt = loadSceneData("D:/CG_resources/classic/cube/cube.obj");
	//auto sceneOpt = loadSceneData("D:/CG_resources/classic/teapot.obj",  SceneLoaderConfig{.scale=0.1f });
	
	if (scene.has_value()) { 
	 
		auto localPaths = scene->texturePaths;
		for (auto& localPath : localPaths) {

			auto texPath = modelDir / localPath;
			auto imageOpt = Loader::LoadImage(texPath.string(),
				Loader::TextureImportConfig{
					.bFlipVOnLoad = true });

			if (imageOpt.has_value()) {
				auto info = imageOpt->metaInfo;
				auto texture = CreateShared<FVkTexture>(deviceRef);
				texture->createTexture(ImageDesc{ info.width, info.height, 4 }, std::move(imageOpt->data));

				this->textures.push_back(texture);
			}
		}


		for (auto& subMesh : scene->meshes)
		{ 
			auto mesh = CreateShared<UStaticMesh>();

			{ 
				cout << "mesh copy:" << '\n';
				Timer<microseconds> LoadTimer;

                mesh->positions = std::move(subMesh.positions);
                mesh->UVs = std::move(subMesh.UVs);
                mesh->indices = std::move(subMesh.indices); 
			} 


			cout << "loaded vertices: " << mesh->positions.size() << '\n';
			cout << "loaded UVs: " << mesh->UVs.size() << '\n';
			cout << "loaded indices: " << mesh->indices.size() << '\n';

			//
			mesh->CreateRHIResource();


			//new:
			glm::mat4 rot= glm::mat4_cast(glm::quat(eulerAngles));
			glm::mat4 trans = glm::translate(glm::mat4(1.0f), translation);
			initPose = rot * trans * glm::mat4(1.0f);

			FStaticMeshObjProxy proxy{
			 .vertexBuffer = mesh->staticMeshResource->vertexBuffer->buffer,
			 .indexBuffer = mesh->staticMeshResource->indexBuffer->buffer,
			 .indexCount = static_cast<uint32_t>(mesh->indices.size()),
			 .indexType = VK_INDEX_TYPE_UINT32,
			 .objectConstants = FObjectConstants{ .model = initPose },
			};
			 

			//new:
			if (this->textures.size() == 0) {
				std::cout << "no texture loaded\n";
			}
			else
			{
				//todo:exprimental: only 0;
				if (scene->materials.size() == 0 ||
					subMesh.materialIndex >= scene->materials.size()) {
					throw std::runtime_error("material loading may have critical issues");
				}
				auto& material = scene->materials[subMesh.materialIndex];

				if (material.textureIndices.size() == 0 ||
					material.textureIndices[0] >= scene->texturePaths.size()) {
					throw std::runtime_error("material loading may have critical issues");
				}
				auto& textureIndex = material.textureIndices[0];

				proxy.texture = textures[textureIndex];
			}
			 
			
			this->meshes.push_back(mesh);
			this->meshProxies.push_back(proxy);
			std::cout << " " << mesh.use_count() << '\n';
		}  

	}

	//test: 
	auto image = Loader::LoadImage("images/image.jpg", 
		Loader::TextureImportConfig{ 
			.bFlipVOnLoad = false});

	if (image.has_value()) { 
		auto info = image->metaInfo; 

		auto texture = CreateShared<FVkTexture>(deviceRef);
		texture->createTexture(ImageDesc{ info.width, info.height, 4 }, std::move(image->data));
		this->debugTexture = texture;
	}




}

