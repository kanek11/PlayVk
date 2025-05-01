

#include "GVulkanRHI.h"

#include "Buffer.h"

GVulkanRHI::~GVulkanRHI()
{ 

	heapManagerRef.reset();
	commandContextRef.reset();

	swapChainRef.reset();

	deviceRef.reset();

	surfaceRef.reset();

	if (enableValidationLayers) {
		destroyDebugUtilsMessengerEXT(vkInstance, vkDebugMessenger, nullptr);
	} 

	vkDestroyInstance(vkInstance, nullptr);

	std::cout << "\tapp: vulkan RHI destroyed!" << '\n'; 
}



GVulkanRHI::GVulkanRHI(WeakPtr<FVkWindow> window)
	:windowRef(window)
{
	 
	populateInstanceExtensions(instanceExtensions); 
	 
	std::cout << "\tCreateInstance: required instance extensions:" << '\n';
	for (const auto& extension : instanceExtensions) {
		std::cout << '\t' << extension << '\n';
	}

	populateDebugMessengerCreateInfo(); 

	if (auto resultOpt = createInstance(); resultOpt.has_value()) {
		this->vkInstance = *resultOpt;
		std::cout << "\tapp: instance created!" << '\n';
	}
	else {
		throw std::runtime_error("Failed to create instance!");
	}

#ifdef _DEBUG
	if (auto resultOpt = createDebugMessenger(); resultOpt.has_value()) {
		this->vkDebugMessenger = *resultOpt;
		std::cout << "\tapp: debug messenger created!" << '\n';
	}
	else {
		throw std::runtime_error("Failed to create debug messenger!");
	}
#endif 
  

	this->surfaceRef = CreateShared<FVkSurface>(vkInstance, windowRef);
	this->deviceRef = CreateShared<FVkDevice>(vkInstance, surfaceRef);
	this->swapChainRef = CreateShared<FVkSwapChain>(deviceRef, windowRef, surfaceRef);
	this->commandContextRef = CreateShared<FVkCommandContext>(deviceRef);
	this->heapManagerRef = CreateShared<FVkHeapManager>(deviceRef);

}




FVkWindow::FVkWindow()
{
	if(init()) 
		std::cout << "window created" << '\n';
	else
		throw std::runtime_error("Failed to create window!");
}

FVkWindow::~FVkWindow()
{
	cleanup();
	std::cout << "window shutdown" << '\n';
}



void  FVkWindow::cleanup()
{
	glfwDestroyWindow(pGLFWWindow);
	glfwTerminate();
}


bool FVkWindow::init() noexcept
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   //disable resize, otherwise need to recreate swapchain

	this->pGLFWWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	if (!pGLFWWindow) {  
		return false;
	}

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
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {

			std::cout << "\tapp: use mailbox mode, present quickest possible" << '\n';
			return availablePresentMode;
		}
	}
	std::cout << "\tapp: use FIFO present mode,  lock to display refresh rate " << '\n';
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D FVkSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		std::cout << "\tapp: use surface current extent" << '\n';
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(windowRef.lock()->pGLFWWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		std::cout << "\tapp:glfw managed extent" << '\n';
		return actualExtent;
	}
}


FVkSurface::~FVkSurface()
{
	vkDestroySurfaceKHR(instanceRef, vkSurface, nullptr);
	std::cout << "\tapp: surface destroyed!" << '\n';
}

FVkSurface::FVkSurface(const VkInstance& instance, const WeakPtr<FVkWindow> window)
	:instanceRef(instance), windowRef(window)
{
	if (auto resultOpt = createSurface(); resultOpt.has_value())
	{
		this->vkSurface = *resultOpt;
		std::cout << "\tapp: surface created!" << '\n';
	}
	else
	{
		throw std::runtime_error("Failed to create surface!");
	}  
}

std::optional<VkSurfaceKHR> FVkSurface::createSurface() const noexcept
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(instanceRef, windowRef.lock()->pGLFWWindow, nullptr, &_surface) != VK_SUCCESS) {
		return std::nullopt;
	}
	else {
		return _surface;
	}
} 


FVkSwapChain::~FVkSwapChain()
{
	auto& device = deviceRef.lock()->vkDevice;

	for (auto& imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, vkSwapChain, nullptr);

	std::cout << "\tapp: swapchain destroyed!" << '\n'; 
}


FVkSwapChain::FVkSwapChain(WeakPtr<FVkDevice> device, WeakPtr<FVkWindow> window, WeakPtr<FVkSurface> surface)
	:deviceRef(device), surfaceRef(surface), windowRef(window)
{
	createSwapChain();
	createSCImageViews();
}


bool FVkSwapChain::createSwapChain()
{ 
	auto& physicalDevice = deviceRef.lock()->vkPhysicalDevice;
	auto& device = deviceRef.lock()->vkDevice; 
	auto& surface = surfaceRef.lock()->vkSurface;


	//SwapChainSupportDetails swapChainSupport = queryDeviceSwapChainSupport(_physicalDevice);
	auto swapChainSupport = deviceRef.lock()->queryDeviceSwapChainSupport(physicalDevice);


	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR }; 
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//QueueFamilyIndices indices = queryQueueFamilies(_physicalDevice);
	auto indices = deviceRef.lock()->queryQueueFamilies(physicalDevice);

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

	this->swapChainImageFormat = surfaceFormat.format;
	this->swapChainExtent = extent;

	std::cout << "\tapp: swapchain created!" << '\n';

	return true;

}



bool FVkSwapChain::createSCImageViews()
{ 
	const auto& device = deviceRef.lock()->vkDevice;

	/*
	*/
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO }; 
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = this->swapChainImageFormat.value();
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

	std::cout << "\tapp: swapchain imageViews created!" << '\n';

	return true;
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




std::optional<VkInstance> GVulkanRHI::createInstance() const 
{

	VkApplicationInfo _appInfo{
		 .sType =  VK_STRUCTURE_TYPE_APPLICATION_INFO,
		 .pApplicationName = "Hello Vulkan",
		 .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		 .pEngineName = "No Engine",
		 .engineVersion = VK_MAKE_VERSION(1, 0, 0),
		 .apiVersion = VK_API_VERSION_1_3,
	};

	VkInstanceCreateInfo _createInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO }; 
	_createInfo.pApplicationInfo = &_appInfo;


	_createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	_createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	 
	//only for debug mode  
	if (enableValidationLayers) {

		if (!queryCoreValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		_createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		_createInfo.ppEnabledLayerNames = validationLayers.data();

		//a dedicated messenger for instance creation and destruction is automatically enabled by passing the pNext chain 
		_createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&m_debugMsgerCreateInfo;

		std::cout << "\tapp: setup DebugMessenger!" << '\n';
	}

	else {
		_createInfo.enabledLayerCount = 0;
		_createInfo.pNext = nullptr;

	} 

	VkInstance _instance;
	if (vkCreateInstance(&_createInfo, nullptr, &_instance) != VK_SUCCESS) {
		return std::nullopt;
	}
	else
	{
		return _instance;
	}

}




std::optional<VkDebugUtilsMessengerEXT> GVulkanRHI::createDebugMessenger() const noexcept
{
	//the messenger that is instance-specific during the lifetime of the instance
	VkDebugUtilsMessengerEXT _debugMessenger;
	if (enableValidationLayers)
		if (CreateDebugUtilsMessengerEXT(vkInstance, &m_debugMsgerCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
			return std::nullopt;
		}
		else {
			return _debugMessenger;
		}
	 
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer callback: " << pCallbackData->pMessage << '\n';

	return VK_FALSE;
}


void  GVulkanRHI::populateDebugMessengerCreateInfo() {
	m_debugMsgerCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	m_debugMsgerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	m_debugMsgerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	m_debugMsgerCreateInfo.pfnUserCallback = debugCallback;
}


bool GVulkanRHI::queryCoreValidationLayerSupport() const
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




std::optional<VkPhysicalDevice> FVkDevice::choosePhysicalDevice() const 
{
	auto instance = instanceRef;  

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


	VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				_physicalDevice = device;
				std::cout << "\tapp: pick discrete GPU: " << deviceProperties.deviceName << '\n';
				break;
			}
			else
			{
				std::cout << "\tapp: skip integrated GPU: " << deviceProperties.deviceName << '\n';
			}
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE ) {
		return std::nullopt;
	}
	 
	//new: 1.1 chains
	enabledDeviceFeatures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	enabledDeviceFeatures2.features.samplerAnisotropy = VK_TRUE;  //enable anisotropy

	dynamicRenderingFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
	//dynamicRenderingFeatures.dynamicRendering = VK_TRUE; // 手动启用动态渲染特性

	enabledDeviceFeatures2.pNext = &dynamicRenderingFeatures;  
	
	VkPhysicalDeviceFeatures2 deviceFeatures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.pNext = &dynamicRenderingFeatures;  //let query populate it
	vkGetPhysicalDeviceFeatures2(_physicalDevice, &deviceFeatures2);

	if (deviceFeatures2.features.samplerAnisotropy == VK_FALSE ||
		dynamicRenderingFeatures.dynamicRendering == VK_FALSE) {
		std::cout << "Feature not supported!" << '\n';
		return std::nullopt;
	}  
	
	return _physicalDevice;
}

 

SwapChainSupportDetails FVkDevice::queryDeviceSwapChainSupport(VkPhysicalDevice device) const {
	SwapChainSupportDetails details;

	auto surface = surfaceRef.lock()->vkSurface;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}



QueueFamilyIndices FVkDevice::queryQueueFamilies(VkPhysicalDevice device) const {

	auto surface = surfaceRef.lock()->vkSurface;

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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				QF_indices.presentFamily = i;
			}
		} 
	
		 
		// we make compute separate from the graphics queue
		if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && 
			!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			QF_indices.computeFamily = i;
		}


		if (QF_indices.isComplete()) {
			break;
		}

		i++;
	}

	if (!QF_indices.isComplete()) {
		std::cout << "no suitable queue family found for this device!" << '\n';
	}


	return QF_indices;
}


bool FVkDevice::isDeviceSuitable(VkPhysicalDevice device) const {
	QueueFamilyIndices indices = queryQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;

	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupportDetails = queryDeviceSwapChainSupport(device);
		swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool FVkDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
		if(strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
			std::cout << "\tapp: device support dynamic rendering" << '\n';
	}

	return requiredExtensions.empty();
}


std::optional<VkDevice> FVkDevice::createLogicalDevice() const
{ 
	auto& physicalDevice = vkPhysicalDevice; 

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { 
		queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() , queueIndices.computeFamily.value()
	};


	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO }; 
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	std::cout << "\tapp: selected queue families for graphics:" << queueIndices.graphicsFamily.value() 
		<< " present:" << queueIndices.presentFamily.value() 
		<< " compute:" << queueIndices.computeFamily.value() << '\n';



	VkDeviceCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO }; 

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

	VkDevice _device;
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
		return std::nullopt;
	}
	else {
		return _device;
	} 
}



FVkCommandContext::~FVkCommandContext()
{
	auto& device = deviceRef.lock()->vkDevice;

	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
	vkDestroyCommandPool(device, computeCommandPool, nullptr); 
	graphicsCommandPool = VK_NULL_HANDLE;
    computeCommandPool = VK_NULL_HANDLE;

	std::cout << "\tapp: command pool destroyed!" << '\n';

}

FVkCommandContext::FVkCommandContext(WeakPtr<FVkDevice> device)
	:deviceRef(device)
{
	getQueues();
	createCommandPools();
}




bool FVkCommandContext::getQueues()
{ 
	auto& device = deviceRef.lock()->vkDevice;
	auto& indices = deviceRef.lock()->queueIndices;

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);

	std::cout << "\tapp: queues created!" << '\n';
	return true;
}


bool FVkCommandContext::createCommandPools()
{
	auto& device = deviceRef.lock()->vkDevice;
	auto& indices = deviceRef.lock()->queueIndices;


	VkCommandPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO }; 
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
	  

	std::cout << "\tapp: command pool created!" << '\n';
	return true;
}



VkCommandBuffer FVkCommandContext::beginGraphicsTransientCmdBuffer() {

	auto& device = deviceRef.lock()->vkDevice;

	VkCommandBufferAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer{};
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;

}


void FVkCommandContext::endGraphicsTransientCmdBuffer(VkCommandBuffer commandBuffer)
{
	auto& device = deviceRef.lock()->vkDevice;

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, graphicsCommandPool, 1, &commandBuffer); 
}


VkCommandBuffer FVkCommandContext::beginComputeTransientCmdBuffer() {

	auto& device = deviceRef.lock()->vkDevice;
	if (computeCommandPool == VK_NULL_HANDLE  )
		throw std::runtime_error("compute command pool not created!");


	VkCommandBufferAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = computeCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;

}


void FVkCommandContext::endComputeTransientCmdBuffer(VkCommandBuffer commandBuffer)
{
	auto& device = deviceRef.lock()->vkDevice;

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(computeQueue);

	vkFreeCommandBuffers(device, computeCommandPool, 1, &commandBuffer);
}






FVkDevice::~FVkDevice() {

	vkDestroyDevice(vkDevice, nullptr);
	std::cout << "\tapp: logical device destroyed!" << '\n'; 
}

FVkDevice::FVkDevice(const VkInstance& instance, const WeakPtr<FVkSurface> surface)
	:instanceRef(instance), surfaceRef(surface)
{
	//
	if (auto resultOpt = choosePhysicalDevice(); resultOpt.has_value()) {
		this->vkPhysicalDevice = *resultOpt;
		std::cout << "\tapp: physical device selected!" << '\n';
	}
	else {
		throw std::runtime_error("Failed to select physical device!");
	} 

	//
	this->queueIndices = queryQueueFamilies(this->vkPhysicalDevice);

	//
	if (auto resultOpt = createLogicalDevice(); resultOpt.has_value()) {
		this->vkDevice = *resultOpt;
		std::cout << "\tapp: logical device created!" << '\n';
	}
	else {
		throw std::runtime_error("Failed to create logical device!");
	}
	 
}





FVkHeapManager::~FVkHeapManager()
{
	std::cout << "\tapp: heap manager destroyed!" << '\n';
}

FVkHeapManager::FVkHeapManager(WeakPtr<FVkDevice> device)
	:deviceRef(device)
{
	std::cout << "\tapp: heap manager created!" << '\n';
}

VkDeviceMemory FVkHeapManager::allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties)
{
	auto& device = deviceRef.lock()->vkDevice;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties).value();

	VkDeviceMemory memory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	std::cout << "\tapp: buffer memory allocated ,size: " << memRequirements.size << '\n';

	return memory;
}

VkDeviceMemory FVkHeapManager::allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties)
{
	auto& device = deviceRef.lock()->vkDevice;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties).value();

	VkDeviceMemory memory;
	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	std::cout << "\tapp: image memory allocated ,size: " << memRequirements.size << '\n';

	return memory;
}







void  FVkHeapManager::updateBufferMemory(VkMemoryPropertyFlags properties, VkBuffer dstBuffer, VkDeviceMemory memory, VkDeviceSize size, const void* data)
{
	//for device local buffer, we need to copy from a host visible ,staging buffer
	if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {

		auto stagingBuffer = CreateShared<FVkBuffer>(deviceRef, FVkBufferDesc{ eSTAGING_BUFFER, size });
		mapMemory(stagingBuffer->memory, size, data);

		copyBuffers(stagingBuffer->buffer, dstBuffer, size);

		std::cout << "begin copy to device local" << '\n';
	}
	else if ( properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		mapMemory(memory, size, data);
		//std::cout << "begin map host visible " << '\n';
	}
	else
	{
		throw std::runtime_error("unsupported memory property!");
	}

	//std::cout << "buffer memory updated!" << '\n';
}


void  FVkHeapManager::updateImageMemory(VkMemoryPropertyFlags properties, VkImage image, VkDeviceMemory memory, VkExtent3D extent, VkDeviceSize size, const void* data)
{
	//for device local buffer, we need to copy from a host visible ,staging buffer
	if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		 
		auto stagingBuffer = CreateShared<FVkBuffer>(deviceRef, FVkBufferDesc{ eSTAGING_BUFFER, size });
		mapMemory(stagingBuffer->memory, size, data);

		copyBufferToImage(stagingBuffer->buffer, image, extent);

		std::cout << "begin copy to device local" << '\n';
	}
	else if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		
		mapMemory(memory, size, data);
		std::cout << "begin map host visible " << '\n';
	}
	else
	{
		throw std::runtime_error("unsupported memory property!");
	}

	std::cout << "buffer memory updated!" << '\n';
}





void FVkHeapManager::mapMemory(VkDeviceMemory memory, VkDeviceSize size, const void* data)
{
	auto& device = deviceRef.lock()->vkDevice;

	void* mappedData;
	vkMapMemory(device, memory, 0, size, 0, &mappedData);
	memcpy(mappedData, data, size);
	vkUnmapMemory(device, memory);

	//std::cout << "\tapp: buffer memory mapped, size: " << size << '\n';
}


void FVkHeapManager::copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	auto& commandContextRef = Global::vulkanRHI->commandContextRef;

	VkCommandBuffer commandBuffer = commandContextRef->beginGraphicsTransientCmdBuffer();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer);
}


void FVkHeapManager::copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D extent)
{
	auto& commandContextRef = Global::vulkanRHI->commandContextRef;

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
	vkGetPhysicalDeviceMemoryProperties(deviceRef.lock()->vkPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!"); 
}