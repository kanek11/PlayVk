#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <optional> 

#include <vector>
#include <set>
#include <iostream> 
#include <format>

#include <algorithm>

#include "Base.h"
#include "VkHandle.h"

//todo
// update buffer is a bit verbose;
// clea n up
// //null check
//visibility wrapping,  set/get 
//extract settable state  out of the boilerplate code

//todo framebuffer


namespace VulkanSettings
{

	inline constexpr bool FIFO_MODE{ false };

	//double buffering
	inline constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };

	//fixed for now;
	inline constexpr uint32_t WIDTH{ 1000 };
	inline constexpr uint32_t HEIGHT{ 800 };


#ifdef NDEBUG
	inline const bool enableValidationLayers{ false };
#else
	inline const bool enableValidationLayers{ true };
#endif

	//provided by Vulkan SDK
	inline const std::vector<const char*> validationLayers {
		"VK_LAYER_KHRONOS_validation"
	};

	inline const std::vector<const char*> deviceExtensions {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	};

	inline std::vector<const char*> instanceExtensions;

	//1.0 core features are in this struct£»
	inline VkPhysicalDeviceFeatures enabledDeviceFeatures{};
	//note pNext chain is not copy, so the struct has to be in the same scope
	inline VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{}; 
	inline VkPhysicalDeviceFeatures2 enabledDeviceFeatures2{};


	//device features  
}

using namespace VulkanSettings;



/*
*/
class FVkWindow
{
public:
	explicit FVkWindow();
	~FVkWindow();

	[[nodiscard]] bool init() noexcept ;
	void cleanup(); 

	GLFWwindow* pGLFWWindow{ nullptr };
};

 

class FVkSurface
{
public:
	~FVkSurface();
	explicit FVkSurface(const VkInstance& instance, const WeakPtr<FVkWindow> window); 

	[[nodiscard]] std::optional<VkSurfaceKHR> createSurface() const noexcept;

	VkSurfaceKHR vkSurface;
private:
	const VkInstance& instanceRef;
	const WeakPtr<FVkWindow> windowRef;
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
	explicit FVkDevice(const VkInstance& instance, const WeakPtr<FVkSurface> surface);


private:
	[[nodiscard]] std::optional<VkPhysicalDevice> choosePhysicalDevice() const;

	[[nodiscard]] std::optional<VkDevice> createLogicalDevice() const; 

public: 
	//todo: make it populate method
	SwapChainSupportDetails queryDeviceSwapChainSupport(VkPhysicalDevice device) const;
	 
	QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device) const;
	 
	//graphics, present queue;  swapchain
	[[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
	[[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;


public:
	//cache the information on logical device creation, after the physical device is selected.
	 QueueFamilyIndices queueIndices;
	//SwapChainSupportDetails swapChainSupportDetails; 

public:
	VkPhysicalDevice vkPhysicalDevice { VK_NULL_HANDLE };
	VkDevice vkDevice { VK_NULL_HANDLE };  

private:
	//readonly reference imply a external dependency
	const VkInstance& instanceRef;
	const WeakPtr<FVkSurface> surfaceRef;
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
	explicit FVkCommandContext(WeakPtr<FVkDevice> device);
	~FVkCommandContext(); 

	bool getQueues();
	bool createCommandPools(); 

public:
	[[nodiscard]] VkCommandBuffer beginGraphicsTransientCmdBuffer();
	void endGraphicsTransientCmdBuffer(VkCommandBuffer commandBuffer);

	[[nodiscard]] VkCommandBuffer beginComputeTransientCmdBuffer();
	void endComputeTransientCmdBuffer(VkCommandBuffer commandBuffer);

public:
	//filled after logical device creation
	VkQueue  graphicsQueue { VK_NULL_HANDLE };
	VkQueue  presentQueue { VK_NULL_HANDLE };
	VkQueue  computeQueue { VK_NULL_HANDLE };

	VkCommandPool graphicsCommandPool { VK_NULL_HANDLE };
	VkCommandPool computeCommandPool { VK_NULL_HANDLE }; 

private:
	const WeakPtr<FVkDevice> deviceRef;
};



/*
* buffer and memory are separated in Vulkan
* we want it to be decoupled from the engine, so pure Vulkan objects for now.
*/
class FVkHeapManager {
public:
	~FVkHeapManager();
	explicit FVkHeapManager(WeakPtr<FVkDevice>);

	VkDeviceMemory allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
	VkDeviceMemory allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties);

	//tod: the usage parse need to be fixed;
	void updateBufferMemory(VkMemoryPropertyFlags properties, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size, const void* data);
	void updateImageMemory(VkMemoryPropertyFlags properties, VkImage image, VkDeviceMemory memory, VkExtent3D extent, VkDeviceSize size, const void* data);

private:
	void mapMemory(VkDeviceMemory memory, VkDeviceSize size, const void* data);
 	void copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D extent);
	 
private:
	std::optional<uint32_t> findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	 
	//track all allocated memory
	//std::vector<VkDeviceMemory> allocatedMemory;

private:
	const WeakPtr<FVkDevice> deviceRef;
};



//todo: resize,  
class FVkSwapChain
{
public:
	~FVkSwapChain();
	explicit FVkSwapChain(WeakPtr<FVkDevice> device, WeakPtr<FVkWindow> window, WeakPtr<FVkSurface> surface); 

	bool createSwapChain(); 
	bool createSCImageViews(); 

	VkSwapchainKHR vkSwapChain { VK_NULL_HANDLE }; 

	//metaInfo cached 
	std::optional<VkExtent2D> swapChainExtent{}; //framebuffer takes resolution
	std::optional<VkFormat> swapChainImageFormat{};  //renderpass needs format,  
	std::optional<VkFormat> depthFormat{ VK_FORMAT_D32_SFLOAT };  //hardcode for now 
 

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	//VkImageView depthImageView;


private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 

private:
	const WeakPtr<FVkDevice> deviceRef;
	const WeakPtr<FVkWindow> windowRef;
	const WeakPtr<FVkSurface> surfaceRef;
};

 

/*
*
* 
* instance-level and management of all Vulkan objects
*
* destroy RHI will clean up all Vulkan objects,
* 
* we init fundamentals here,
* task-related objects are created separately. 

*/

class GVulkanRHI
{
public:
	~GVulkanRHI();  //clean up all RHI resources, todo
	explicit GVulkanRHI(WeakPtr<FVkWindow> window);

private: 
	bool populateInstanceExtensions(std::vector<const char*>&);

	[[nodiscard]] std::optional<VkInstance> createInstance() const ;
	[[nodiscard]] std::optional<VkDebugUtilsMessengerEXT> createDebugMessenger() const noexcept; 

	//utils
	bool queryCoreValidationLayerSupport() const;
	 
	VkDebugUtilsMessengerCreateInfoEXT m_debugMsgerCreateInfo{};
	void populateDebugMessengerCreateInfo();

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	} 

public:
	VkInstance vkInstance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT vkDebugMessenger; 

	SharedPtr<FVkSurface> surfaceRef;
	SharedPtr<FVkDevice> deviceRef;  //device depend on surface 
	SharedPtr<FVkSwapChain> swapChainRef; 
	SharedPtr<FVkCommandContext> commandContextRef;
	SharedPtr<FVkHeapManager> heapManagerRef;  //copy data depend on cmd context

	//expect from outside
	const WeakPtr<FVkWindow> windowRef;
};






//global variable 
namespace Global { 
    /*
	* this is the only exposed raw ptr i will use in the engine,
	* C++ 17 inline so linker so it's ok if only declared in one header.
	*/
	inline GVulkanRHI* vulkanRHI = nullptr;

}
