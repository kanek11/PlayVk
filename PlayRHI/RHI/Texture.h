#pragma once


#include "GVulkanRHI.h"

#include "unordered_map"

/*
* as agnostic as possible, pure data aspect of image;
*/
struct ImageMetaInfo {

   uint32_t width, height ;
   uint32_t channels; 
};


enum class EImageUsage {

	SAMPLER,
	COLOR_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT, 
    SHADOW_MAP, 
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

	void CreateTexture(ImageMetaInfo& imageInfo, void* data); 
	void CreateFBDepthTexture(uint32_t width, uint32_t height, VkFormat format);


	void CreateVkImage(); 
	void CreateImageView();
	void CreateSampler();  

	void GenerateMipmaps();
	

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


void LoadImage(const char* path, ImageMetaInfo& imageInfo, void*& data);
