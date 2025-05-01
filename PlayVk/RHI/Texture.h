#pragma once


#include "GVulkanRHI.h"

#include <unordered_map>
#include <functional>

/*
* as agnostic as possible, pure data aspect of image;
*/
struct ImageDesc { 
   uint32_t width{}, height{};
   uint32_t channels{};
}; 

enum class EImageUsage {

	eSAMPLER,
	eCOLOR_ATTACHMENT,
    eDEPTH_STENCIL_ATTACHMENT, 
    eSHADOW_MAP, 
};

//open for extension:
struct FVkImageConfig {

	VkExtent3D extent{};
	VkFormat format{};
	VkImageUsageFlags usage{};
	VkMemoryPropertyFlags memoryProperty{};

	VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
};
 

struct FVkImageStageConfig {

	VkPipelineStageFlags stage{};
	VkImageLayout layout{};
    VkAccessFlags access{};
}; 
 

class FVkTexture { 
public: 
	using ImageHandle = std::unique_ptr<void, std::function<void(void*)>>;

	~FVkTexture();
	explicit FVkTexture(const WeakPtr<FVkDevice>& device) : deviceRef(device){ }

	FVkTexture(const FVkTexture&) = delete;
	FVkTexture& operator=(const FVkTexture&) = delete;

	FVkTexture(FVkTexture&&) = delete;
	FVkTexture& operator=(FVkTexture&&) = delete;

	//factorys
	void createTexture(ImageDesc imageInfo, ImageHandle dataHandle);
	void createFBDepthTexture(uint32_t width, uint32_t height, VkFormat format);
	void createStorageImage(uint32_t width, uint32_t height);

private:
	void createVkImage(); 
	void createImageView();
	void createSampler();  

	void generateMipmaps();
	
public:
	//expect to init the information before creating the texture; 
	std::optional<FVkImageConfig> imageConfig{};
	uint32_t mipLevels{ 1 };
	bool bGenerateMipmaps{ false };

	VkImage image { VK_NULL_HANDLE };
	VkDeviceMemory imageMemory { VK_NULL_HANDLE };
	VkImageView imageView { VK_NULL_HANDLE };
	VkSampler sampler { VK_NULL_HANDLE };
private:
	const WeakPtr<FVkDevice> deviceRef;
};

 
