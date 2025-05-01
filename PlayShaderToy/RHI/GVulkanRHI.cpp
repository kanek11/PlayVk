

#include "GVulkanRHI.h"

#include "Buffer.h"

GVulkanRHI::~GVulkanRHI()
{

}



GVulkanRHI::GVulkanRHI(SharedPtr<FVkWindow> window)
	:windowRef(window)
{

	createInstance();
	createDebugMessenger();

	surfaceRef = CreateShared<FVkSurface>(vkInstance, windowRef);
	deviceRef = CreateShared<FVkDevice>(vkInstance, surfaceRef);
	swapChainRef = CreateShared<FVkSwapChain>(deviceRef, windowRef, surfaceRef);


	commandContextRef = CreateShared<FVkCommandContext>(deviceRef);
	heapManagerRef = CreateShared<FVkHeapManager>(deviceRef);

}




FVkWindow::FVkWindow()
{
	init();
}

FVkWindow::~FVkWindow()
{
	cleanup();
}



void  FVkWindow::cleanup()
{
	glfwDestroyWindow(pGLFWWindow);
	glfwTerminate();
}


bool FVkWindow::init()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   //disable resize, otherwise need to recreate swapchain

	pGLFWWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	if (!pGLFWWindow) {
		throw std::runtime_error("failed to create window!");

		return false;
	}

	std::cout << "\tapp: glfw window created!" << std::endl;
	return true;
}





//turns out SRGB is not so compatible with "storage bit",  just encode it in the shader manually;
VkSurfaceFormatKHR FVkSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR FVkSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	if(!FIFO_MODE)
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {

			std::cout << "\tapp: use mailbox mode, present quickest possible" << std::endl;
			return availablePresentMode;
		}
	}
	std::cout << "\tapp: use FIFO present mode,  lock to display refresh rate " << std::endl;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D FVkSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		std::cout << "\tapp: use surface current extent" << std::endl;
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(windowRef->pGLFWWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		std::cout << "\tapp:glfw managed extent" << std::endl;
		return actualExtent;
	}
}


FVkSurface::~FVkSurface()
{
}

FVkSurface::FVkSurface(const VkInstance& instance, const SharedPtr<FVkWindow> window)
	:instanceRef(instance), windowRef(window)
{
	createSurface();
}


bool FVkSurface::createSurface()
{
	if (glfwCreateWindowSurface(instanceRef, windowRef->pGLFWWindow, nullptr, &vkSurface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
	std::cout << "\tapp: surface created!" << std::endl;
	return true;
}



FVkSwapChain::~FVkSwapChain()
{

}


FVkSwapChain::FVkSwapChain(SharedPtr<FVkDevice> device, SharedPtr<FVkWindow> window, SharedPtr<FVkSurface> surface)
	:deviceRef(device), surfaceRef(surface), windowRef(window)
{
	createSwapChain();
	createSCImageViews();
}


bool FVkSwapChain::createSwapChain()
{

	auto physicalDevice = deviceRef->vkPhysicalDevice;
	auto device = deviceRef->vkDevice;


	//SwapChainSupportDetails swapChainSupport = queryDeviceSwapChainSupport(_physicalDevice);
	auto swapChainSupport = deviceRef->queryDeviceSwapChainSupport(physicalDevice);


	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR }; 
	createInfo.surface = surfaceRef->vkSurface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//QueueFamilyIndices indices = queryQueueFamilies(_physicalDevice);
	auto indices = deviceRef->queryQueueFamilies(physicalDevice);

	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	//store the swapchain images
	vkGetSwapchainImagesKHR(device, vkSwapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, vkSwapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	std::cout << "\tapp: swapchain created!" << std::endl;

}



bool FVkSwapChain::createSCImageViews()
{
	/**/
	auto device = deviceRef->vkDevice;

	/*
	*/
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}

	std::cout << "\tapp: swapchain imageViews created!" << std::endl;

}







/*
* an extension is not loaded automatically like "core" Vulkan functions,
* we must load it manually using vkGetInstanceProcAddr and serve as a validation if the extension is available.
* then we return the X()  like normal binding workflow.
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}




bool GVulkanRHI::createInstance()
{
	std::cout << "Begin create instance" << std::endl;

	populateInstanceExtensions(instanceExtensions);
	populateDebugMessengerCreateInfo();

	//
	std::cout << "\tCreateInstance: required instance extensions:" << std::endl;
	for (const auto& extension : instanceExtensions) {
		std::cout << '\t' << extension << std::endl;
	}


	VkApplicationInfo _appInfo{
		 .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		 .pApplicationName = "Hello Vulkan",
		 .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		 .pEngineName = "No Engine",
		 .engineVersion = VK_MAKE_VERSION(1, 0, 0),
		 .apiVersion = VK_API_VERSION_1_3,
	};

	VkInstanceCreateInfo _createInfo{};
	_createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_createInfo.pApplicationInfo = &_appInfo;


	_createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	_createInfo.ppEnabledExtensionNames = instanceExtensions.data();






	//only for debug mode  
	if (enableValidationLayers) {

		if (!queryVkCoreValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		_createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		_createInfo.ppEnabledLayerNames = validationLayers.data();

		//a dedicated messenger for instance creation and destruction is automatically enabled by passing the pNext chain 
		_createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&m_debugMsgerCreateInfo;

		std::cout << "\tapp: setup DebugMessenger!" << std::endl;
	}

	else {
		_createInfo.enabledLayerCount = 0;
		_createInfo.pNext = nullptr;

	}

	if (vkCreateInstance(&_createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	std::cout << "\tapp: instance created!" << std::endl;
	return true;

}




bool GVulkanRHI::createDebugMessenger()
{
	//the messenger that is instance-specific during the lifetime of the instance
	if (enableValidationLayers)
		if (CreateDebugUtilsMessengerEXT(vkInstance, &m_debugMsgerCreateInfo, nullptr, &vkDebugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}

	return true;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer callback: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


void  GVulkanRHI::populateDebugMessengerCreateInfo() {
	m_debugMsgerCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	m_debugMsgerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	m_debugMsgerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	m_debugMsgerCreateInfo.pfnUserCallback = debugCallback;
}


bool GVulkanRHI::queryVkCoreValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}


bool GVulkanRHI::populateInstanceExtensions(std::vector<const char*>& instanceExtensions)
{

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	instanceExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return true;
}





bool FVkDevice::ChoosePhysicalDevice()
{
	auto instance = instanceRef;


	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				vkPhysicalDevice = device;
				std::cout << "\tapp: pick discrete GPU: " << deviceProperties.deviceName << std::endl;
				break;
			}
			else
			{
				std::cout << "\tapp: skip integrated GPU: " << deviceProperties.deviceName << std::endl;
			}
		}
	}

	if (vkPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}



	//new: 1.1 chains
	enabledDeviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	enabledDeviceFeatures2.features.samplerAnisotropy = VK_TRUE;  //enable anisotropy

	dynamicRenderingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
	//dynamicRenderingFeatures.dynamicRendering = VK_TRUE; // 手动启用动态渲染特性

	enabledDeviceFeatures2.pNext = &dynamicRenderingFeatures; 

	
	VkPhysicalDeviceFeatures2 deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.pNext = &dynamicRenderingFeatures;  //let query populate it
	vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &deviceFeatures2);

	if (deviceFeatures2.features.samplerAnisotropy == VK_FALSE  ) {
		throw std::runtime_error("Feature Sampler Anisotropy not supported!");
	}
	if (dynamicRenderingFeatures.dynamicRendering == VK_FALSE) {
		throw std::runtime_error("Dynamic Rendering is not supported ");
	}

	std::cout << "\tapp: end pick physical device" << std::endl;
}







SwapChainSupportDetails FVkDevice::queryDeviceSwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surfaceRef->vkSurface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceRef->vkSurface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceRef->vkSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceRef->vkSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceRef->vkSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}



QueueFamilyIndices FVkDevice::queryQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices QF_indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); 


	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		//graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			QF_indices.graphicsFamily = i;

		   //make sure the same with present
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surfaceRef->vkSurface, &presentSupport);

			if (presentSupport) {
				QF_indices.presentFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				QF_indices.computeFamily = i;
			}
		}  
		if (QF_indices.isComplete()) {
			break;
		}

		i++;
	}

	if (!QF_indices.isComplete()) {
		std::cout << "no suitable queue family found for this device!" << std::endl;
	}


	return QF_indices;
}


bool FVkDevice::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = queryQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupportDetails = queryDeviceSwapChainSupport(device);
		swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool FVkDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
		if(strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
			std::cout << "\tapp: device support dynamic rendering" << std::endl;
	}

	return requiredExtensions.empty();
}



bool FVkDevice::createLogicalDevice()
{

	auto physicalDevice = vkPhysicalDevice; 

	this->queueIndices = queryQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { 
		queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() , queueIndices.computeFamily.value()
	};


	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO }; 
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	std::cout << "\tapp: selected queue families for graphics:" << queueIndices.graphicsFamily.value() 
		<< " present:" << queueIndices.presentFamily.value() 
		<< " compute:" << queueIndices.computeFamily.value() << std::endl;



	VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO }; 

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();  


	createInfo.pEnabledFeatures = nullptr;// &enabledDeviceFeatures;
	createInfo.pNext = &enabledDeviceFeatures2; 
	

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}


	std::cout << "\tapp: logical device created!" << std::endl;




}



FVkCommandContext::~FVkCommandContext()
{
}

FVkCommandContext::FVkCommandContext(SharedPtr<FVkDevice> device)
	:deviceRef(device)
{
	getQueues();
	createCommandPools();
}




bool FVkCommandContext::getQueues()
{ 
	auto device = deviceRef->vkDevice;
	auto indices = deviceRef->queueIndices;

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);

	std::cout << "\tapp: queues created!" << std::endl;
	return true;
}


bool FVkCommandContext::createCommandPools()
{
	auto device = deviceRef->vkDevice;
	auto indices = deviceRef->queueIndices;


	VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO }; 
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //recommand over reset the entire pool
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	//compute pool
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //recommand over reset the entire pool
	poolInfo.queueFamilyIndex = indices.computeFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
	  

	std::cout << "\tapp: command pool created!" << std::endl;
	return true;
}



VkCommandBuffer FVkCommandContext::beginGraphicsTransientCmdBuffer() {

	auto device = deviceRef->vkDevice; 

	VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;

}


void FVkCommandContext::endGraphicsTransientCmdBuffer(VkCommandBuffer commandBuffer)
{
	auto device = deviceRef->vkDevice;

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, graphicsCommandPool, 1, &commandBuffer); 
}


VkCommandBuffer FVkCommandContext::beginComputeTransientCmdBuffer() {

	auto device = deviceRef->vkDevice;
	if (computeCommandPool == VK_NULL_HANDLE)
		throw std::runtime_error("compute command pool not created!");


	VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = computeCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;

}


void FVkCommandContext::endComputeTransientCmdBuffer(VkCommandBuffer commandBuffer)
{
	auto device = deviceRef->vkDevice;

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(computeQueue);

	vkFreeCommandBuffers(device, computeCommandPool, 1, &commandBuffer);
}






FVkDevice::~FVkDevice() {

}

FVkDevice::FVkDevice(const VkInstance& instance, const SharedPtr<FVkSurface> surface)
	:instanceRef(instance), surfaceRef(surface)
{
	ChoosePhysicalDevice();
	createLogicalDevice();
}





FVkHeapManager::~FVkHeapManager()
{
}

FVkHeapManager::FVkHeapManager(SharedPtr<FVkDevice> device)
	:deviceRef(device)
{
	std::cout << "\tapp: heap manager created!" << std::endl;
}

VkDeviceMemory FVkHeapManager::allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties)
{
	auto device = deviceRef->vkDevice;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties).value();

	VkDeviceMemory memory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	std::cout << "\tapp: buffer memory allocated ,size: " << memRequirements.size << std::endl;

	return memory;
}

VkDeviceMemory FVkHeapManager::allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties)
{
	auto device = deviceRef->vkDevice;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties).value();

	VkDeviceMemory memory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	std::cout << "\tapp: image memory allocated ,size: " << memRequirements.size << std::endl;

	return memory;
}







void  FVkHeapManager::updateBufferMemory(VkMemoryPropertyFlags properties, VkBuffer dstBuffer, VkDeviceMemory memory, VkDeviceSize size, const void* data)
{
	//for device local buffer, we need to copy from a host visible ,staging buffer
	if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {

		auto stagingBuffer = CreateShared<FVkBuffer>(deviceRef, eSTAGING_BUFFER, size); 
		mapMemory(stagingBuffer->memory, size, data);

		copyBuffers(stagingBuffer->buffer, dstBuffer, size);

		std::cout << "begin copy to device local" << std::endl;
	}
	else if ( properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		mapMemory(memory, size, data);
		//std::cout << "begin map host visible " << std::endl;
	}
	else
	{
		throw std::runtime_error("unsupported memory property!");
	}

	//std::cout << "buffer memory updated!" << std::endl;
}


void  FVkHeapManager::updateImageMemory(VkMemoryPropertyFlags properties, VkImage image, VkDeviceMemory memory, VkExtent3D extent, VkDeviceSize size, const void* data)
{
	//for device local buffer, we need to copy from a host visible ,staging buffer
	if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		 
		auto stagingBuffer = CreateShared<FVkBuffer>(deviceRef, eSTAGING_BUFFER, size);
		mapMemory(stagingBuffer->memory, size, data);

		copyBufferToImage(stagingBuffer->buffer, image, extent);

		std::cout << "begin copy to device local" << std::endl;
	}
	else if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		
		mapMemory(memory, size, data);
		std::cout << "begin map host visible " << std::endl;
	}
	else
	{
		throw std::runtime_error("unsupported memory property!");
	}

	std::cout << "buffer memory updated!" << std::endl;
}





void FVkHeapManager::mapMemory(VkDeviceMemory memory, VkDeviceSize size, const void* data)
{
	auto device = deviceRef->vkDevice;

	void* mappedData;
	vkMapMemory(device, memory, 0, size, 0, &mappedData);
	memcpy(mappedData, data, size);
	vkUnmapMemory(device, memory);

	//std::cout << "\tapp: buffer memory mapped, size: " << size << std::endl;
}


void FVkHeapManager::copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	auto commandContextRef = Global::vulkanRHI->commandContextRef;

	VkCommandBuffer commandBuffer = commandContextRef->beginGraphicsTransientCmdBuffer();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer);
}


void FVkHeapManager::copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D extent)
{
	auto commandContextRef = Global::vulkanRHI->commandContextRef;

	VkCommandBuffer commandBuffer = commandContextRef->beginGraphicsTransientCmdBuffer();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0; 
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = extent.depth;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = extent;

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer);

}




 




std::optional<uint32_t> FVkHeapManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(deviceRef->vkPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
	return std::nullopt;
}