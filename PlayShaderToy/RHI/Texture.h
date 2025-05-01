#pragma once


#include "GVulkanRHI.h"

#include "unordered_map"

/*
* as agnostic as possible, pure data aspect of image;
*/
struct ImageDesc {

   uint32_t width, height ;
   uint32_t channels; 
}; 

enum class EImageUsage {

	eSAMPLER,
	eCOLOR_ATTACHMENT,
    eDEPTH_STENCIL_ATTACHMENT, 
    eSHADOW_MAP, 
};

//open for extension:
struct FVkImageConfig {

	VkExtent3D extent;
	VkFormat format;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags memoryProperty;

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
};
 

struct FVkImageStageConfig {

	VkPipelineStageFlags stage;
	VkImageLayout layout;
    VkAccessFlags access;
}; 
 

class FVkTexture { 
public: 
	FVkTexture(const SharedPtr<FVkDevice>& device) : deviceRef(device){
	}

	void createTexture(ImageDesc& imageInfo, void* data); 
	void createFBDepthTexture(uint32_t width, uint32_t height, VkFormat format);
	void createStorageImage(uint32_t width, uint32_t height);

	void createVkImage(); 
	void createImageView();
	void createSampler();  

	void generateMipmaps();
	

	//expect to init the information before creating the texture; 
	FVkImageConfig imageConfig;  
	uint32_t mipLevels = 1;
	void* data;

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
private:
	const SharedPtr<FVkDevice> deviceRef;
};


void LoadImage(const char* path, ImageDesc& imageInfo, void*& data);
