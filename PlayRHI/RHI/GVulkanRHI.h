#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <optional> 

#include <vector>
#include <set>
#include <iostream> 

#include <algorithm>

#include "Base.h"

//todo
//null check
//visibility wrapping,  set/get 
//extract settable state  out of the boilerplate code

//todo framebuffer


namespace VulkanSettings
{
	//double buffering
	inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	//fixed for now;
	inline constexpr uint32_t WIDTH = 1000;
	inline constexpr uint32_t HEIGHT = 800;


#ifdef NDEBUG
	inline const bool enableValidationLayers = false;
#else
	inline const bool enableValidationLayers = true;
#endif

	//provided by Vulkan SDK
	inline const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	inline const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	};

	inline std::vector<const char*> instanceExtensions;

	//1.0 core features are in this struct£»
	inline VkPhysicalDeviceFeatures enabledDeviceFeatures = {};
	//note pNext chain is not copy, so the struct has to be in the same scope
	inline VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures  = {}; 
	inline VkPhysicalDeviceFeatures2 enabledDeviceFeatures2 = {};


	//device features  
}

using namespace VulkanSettings;



/*
*

*/
class FVkWindow
{
public:
	FVkWindow();
	~FVkWindow();

	bool Init();
	void Cleanup();




	GLFWwindow* pGLFWWindow = nullptr;
};




class FVkSurface
{
public:
	~FVkSurface();
	FVkSurface(const VkInstance& instance, const SharedPtr<FVkWindow> window); 

	bool CreateSurface(); 

	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

private:
	const VkInstance& instanceRef;
	const SharedPtr<FVkWindow> windowRef;
};







struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};


//group swapchain creation info
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;   //query window
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/*
*
Device is the gateway to the GPU driver,
it's the "context" of vk commands,

device is responsible for,
what vk expose to host, but too low-level to be exposed to the engine user.

such as
pool, queue,   cache,
memory management, reusing,

*/



class FVkDevice
{
public:
	~FVkDevice();
	FVkDevice(const VkInstance& instance, const SharedPtr<FVkSurface> surface);


	bool ChoosePhysicalDevice();

	bool CreateLogicalDevice();


public: 
	//todo: make it populate method
	SwapChainSupportDetails queryDeviceSwapChainSupport(VkPhysicalDevice device);
	 
	QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);

public:
	//graphics, present queue;  swapchain
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);


public:
	//cache the information on logical device creation, after the physical device is selected.
	 QueueFamilyIndices queueIndices;
	//SwapChainSupportDetails swapChainSupportDetails; 


	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vkDevice = VK_NULL_HANDLE;

	const SharedPtr<FVkSurface> surfaceRef;

private:
	//readonly reference imply a external dependency
	const VkInstance& instanceRef;
};





/*
* the API basically involves two kinda of functions,
* 1. for the driver ,  about the management and preparation
* 2. for the GPU, involves actual data transfer, render tasks,
* we wrap the command context that actually submit commands to the GPU
*
* todo: maybe introduce more advanced command buffer management;
*

*/

class FVkCommandContext {
public:
	FVkCommandContext(SharedPtr<FVkDevice> device);
	~FVkCommandContext();


	bool GetQueues();
	bool CreateCommandPools(); 

public:
	VkCommandBuffer BeginGraphicsTransientCmdBuffer();
	void EndGraphicsTransientCmdBuffer(VkCommandBuffer commandBuffer);


public:
	//filled after logical device creation
	VkQueue  graphicsQueue = VK_NULL_HANDLE;
	VkQueue  presentQueue = VK_NULL_HANDLE;
	VkQueue  computeQueue = VK_NULL_HANDLE;

	VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
	VkCommandPool computeCommandPool = VK_NULL_HANDLE; 

private:
	const SharedPtr<FVkDevice> deviceRef;
};



/*
* buffer and memory are separated in Vulkan
* we want it to be decoupled from the engine, so pure Vulkan objects for now.
*/
class FVkHeapManager {
public:
	~FVkHeapManager();
	FVkHeapManager(SharedPtr<FVkDevice>);

	VkDeviceMemory AllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
	VkDeviceMemory AllocateImageMemory(VkImage image, VkMemoryPropertyFlags properties);

	//tod: the usage parse need to be fixed;
	void UpdateBufferMemory(VkMemoryPropertyFlags properties, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size, const void* data);
	void UpdateImageMemory(VkMemoryPropertyFlags properties, VkImage image, VkDeviceMemory memory, VkExtent3D extent, VkDeviceSize size, const void* data);

private:
	void MapMemory(VkDeviceMemory memory, VkDeviceSize size, const void* data);
 	void CopyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D extent);


private:
	std::optional<uint32_t> FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


	//track all allocated memory
	//std::vector<VkDeviceMemory> allocatedMemory;

private:
	const SharedPtr<FVkDevice> deviceRef;
};





class FVkSwapChain
{
public:
	~FVkSwapChain();
	FVkSwapChain(SharedPtr<FVkDevice> device, SharedPtr<FVkWindow> window, SharedPtr<FVkSurface> surface);


	bool CreateSwapChain();

	bool CreateSCImageViews();


	VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;

	//metaInfo cached  
	//  renderpass takes format,  
   // framebuffer takes resolution,   analogous to buffer size 
 
	VkExtent2D swapChainExtent;
	VkFormat swapChainImageFormat;
	//VkFormat depthFormat; 


	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	//VkImageView depthImageView;


private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);



private:
	const SharedPtr<FVkDevice> deviceRef;
	const SharedPtr<FVkWindow> windowRef;
	const SharedPtr<FVkSurface> surfaceRef;
};





/*
*
* high-level  that
* instance-level and management of all Vulkan objects
*
* we init device, swapchain here ,  those fundamental to have.
*
* but task-related objects can be created separately.

*/
class GVulkanRHI
{
public:
	~GVulkanRHI();  //clean up all RHI resources, todo
	GVulkanRHI(SharedPtr<FVkWindow> window);


	bool CreateInstance();

	bool CreateDebugMessenger();



public:
	VkInstance vkInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;


	SharedPtr<FVkSurface> surfaceRef;
	SharedPtr<FVkDevice> deviceRef;  //device depend on surface 
	SharedPtr<FVkSwapChain> swapChainRef;

	SharedPtr<FVkCommandContext> commandContextRef;
	SharedPtr<FVkHeapManager> heapManagerRef;  //copy data depend on cmd context

	//expect from outside
	SharedPtr<FVkWindow> windowRef; 

private:
	bool queryVkCoreValidationLayerSupport();
	bool populateInstanceExtensions(std::vector<const char*>&);


private:
	VkDebugUtilsMessengerCreateInfoEXT m_debugMsgerCreateInfo{};
	void populateDebugMessengerCreateInfo();
};






//global variable 
namespace Global {

	inline GVulkanRHI* vulkanRHI = nullptr;
	//C++ 17 inline so linker doesn't yell.  it's ok if only declared in one header.
}
