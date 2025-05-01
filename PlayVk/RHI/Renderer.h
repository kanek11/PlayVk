#pragma once  
#include <filesystem>

#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h"

#include "Mesh.h" 

#include "Texture.h"

#include "ComputePipeline.h"

#include "Pipeline.h"

//todo: 
//revisit:  
//variant for a renderpass
//a object needs exactly what, for the renderer?

//using resourceBinding = std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>;



//per-frame data
struct alignas(16) FCameraUBO {
	glm::mat4 view;
	glm::mat4 proj;  
	//alignas(16) glm::vec3 cameraPos; 
};

//<=128 bytes
struct alignas(16) FObjectConstants {
	glm::mat4 model; 
};


struct FStaticMeshObjProxy {
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	uint32_t indexCount;
	VkIndexType indexType = VK_INDEX_TYPE_UINT32;
	uint32_t instanceCount = 1;
	//per-object shader data 
	std::optional<FObjectConstants> objectConstants; 
	std::weak_ptr<FVkTexture> texture;
	std::array<std::vector<VkDescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorSets;
};


/*
* the rendering loop 
* and all decisions should be delayed until runtime
*/ 
class FVkRenderer { 
public: 
	FVkRenderer();
	~FVkRenderer();

	void init();
	void shutdown();
	void update() noexcept; 
	void render() noexcept;
	void recordRenderCommand(VkCommandBuffer graphicsCmdBuffer, uint32_t imageIndex) noexcept;


	void createCommandBuffers();
	void createSyncObjects();

	void createSCFramebuffers(); 

	//potential to be virtual;
	void createSceneResources();
	void initBindings();

	//system managed
	//===========================
	SharedPtr<FVkWindow> window;
	WeakPtr<FVkDevice> deviceRef;
	WeakPtr<FVkCommandContext> commandContextRef;
	WeakPtr<FVkHeapManager> heapManagerRef;

	// 
	WeakPtr<FVkSwapChain> swapChainRef; 

	std::vector<VkFramebuffer> swapChainFramebuffers;
	SharedPtr<FVkTexture> FBDepthTexture;

	//depend on application  
    //scene  

	//object level
	std::vector<SharedPtr<UStaticMesh>> meshes;
	std::vector<FStaticMeshObjProxy> meshProxies; 
	 
	std::vector<SharedPtr<FVkTexture>> textures;

	//pipeline level
	SharedPtr<FVkShaderMap> shaderManager; 

	//system level 
	SharedPtr<FVkTexture> debugTexture;
	SharedPtr<FVkBuffer> cameraUBO; 


	//render pipeline specific
	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> graphicsCommandBuffers;

	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> renderInFlightFences; 

	 
	SharedPtr<FVkGraphicsPipeline> graphicsPipeline;
	//SharedPtr<FVkComputePipeline> computePipeline;
	//std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeFinishedSemaphores;


    uint32_t currentFrame{ 0 };
	//new:
	double deltaTime{}; 

	//tweaks
	//A2
	//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -2.0f);
	//glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);
	//vampire
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f);
	glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);

	std::filesystem::path modelDir{};
	std::filesystem::path modelName{}; 
	
	//glm::vec3 eulerAngles{ glm::radians(180.0f), glm::radians(0.0f), glm::radians(0.0f) }; // Rotate 90 degrees around Y-axis
    //glm::vec3 translation{ 0.0f, 1.0f, 0.0f };
	glm::vec3 eulerAngles{ glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f) }; 
	glm::vec3 translation{ 0.0f, -1.0f, 0.0f };


	glm::mat4 initPose = glm::mat4(1.0f);

	float scale = 1.0f;
};