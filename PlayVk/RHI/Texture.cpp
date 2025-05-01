#include "Texture.h"

//todo:  handle depth format, storage image format ;  
//todo:  control generate mipmap and max levels

VkFormat GetVulkanFormatLDR(uint32_t channels) {
	switch (channels) {
	case 1: // Grayscale
		return VK_FORMAT_R8_UNORM;
	case 2: // Grayscale + Alpha
		return VK_FORMAT_R8G8_UNORM;
	case 3: // RGB
		return VK_FORMAT_R8G8B8_UNORM;
	case 4: // RGBA
		return VK_FORMAT_R8G8B8A8_UNORM;
	default:
		throw std::invalid_argument("Unsupported number of channels");
	}
}


VkFormat GetVkFormatHDR(uint32_t channels) {
	switch (channels) {
	case 1: // Grayscale
		return VK_FORMAT_R32_SFLOAT;
	case 2: // Grayscale + Alpha
		return VK_FORMAT_R32G32_SFLOAT;
	case 3: // RGB
		return VK_FORMAT_R32G32B32_SFLOAT;
	case 4: // RGBA
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		throw std::invalid_argument("Unsupported number of channels");
	}
}

 



void InsertImageBarrier(VkCommandBuffer commandBuffer, VkImage image, 
	FVkImageStageConfig& srcStage, FVkImageStageConfig& dstStage,
	 uint32_t mipLevels =1) { 

	auto oldLayout = srcStage.layout;
	auto newLayout = dstStage.layout;
	auto sourceStage = srcStage.stage;
	auto destinationStage = dstStage.stage;
	auto srcAccess = srcStage.access;
	auto dstAccess = dstStage.access;


	VkImageMemoryBarrier barrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

	barrier.image = image;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;


	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;


	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

} 



FVkTexture::~FVkTexture()
{  
	if (!deviceRef.expired()) {
		auto& device = deviceRef.lock()->vkDevice;

		vkDestroySampler(device, sampler, nullptr);
		vkDestroyImageView(device, imageView, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, imageMemory, nullptr); 

		std::cout << "texture destroyed" << '\n';
	} 
	else {
		std::cerr<< "destroy texture after device!" << '\n';
	}

}

void FVkTexture::createTexture(ImageDesc imageInfo, ImageHandle dataHandle) {

	auto& device = deviceRef.lock()->vkDevice;
	auto& heapManager = Global::vulkanRHI->heapManagerRef;
	auto& commandContextRef = Global::vulkanRHI->commandContextRef;

	//validation:
	if (imageInfo.channels == 3) {
		std::cerr << "Texture: RGB has poor support for tiling optimization! " << '\n';
		return;
	}

	//hardcode for now
	//both src and dst if we generate mipmaps ; then it's also transfer src;
	this->imageConfig = {
		{imageInfo.width, imageInfo.height, 1},
		GetVulkanFormatLDR(imageInfo.channels),
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
	};

	if (bGenerateMipmaps)//off for now;
	{
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageInfo.width, imageInfo.width)))) + 1;
		std::cout << "mipLevels: " << mipLevels << '\n';
	 }


	auto& imageConfig = this->imageConfig.value(); 

	//describe the transitions 
	FVkImageStageConfig initStage{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 0 };
	FVkImageStageConfig transferStage{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT };
	FVkImageStageConfig renderStage{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT };


	createVkImage();
	this->imageMemory = heapManager->allocateImageMemory(image, imageConfig.memoryProperty);
	vkBindImageMemory(device, image, imageMemory, 0);


	VkDeviceSize imageSize = imageConfig.extent.width * imageConfig.extent.height * 4; //todo:don't hardcode 4 channels

	//transfer the base level
	VkCommandBuffer commandBuffer1 = commandContextRef->beginGraphicsTransientCmdBuffer();
	InsertImageBarrier(commandBuffer1, image, initStage, transferStage, mipLevels); 
	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer1);

	heapManager->updateImageMemory(imageConfig.memoryProperty, image, imageMemory, imageConfig.extent, imageSize, dataHandle.get());

	 if(bGenerateMipmaps)  
	   generateMipmaps();
	else
	{
	 VkCommandBuffer commandBuffer2 = commandContextRef->beginGraphicsTransientCmdBuffer();
	 InsertImageBarrier(commandBuffer2, image, transferStage, renderStage);
	 commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer2); 
	}
	


	createImageView();
	createSampler(); 
}




 

void FVkTexture::createVkImage()
{
	auto& device = deviceRef.lock()->vkDevice; 

	auto& imageConfig = *(this->imageConfig);
	auto usage = imageConfig.usage;
	auto format = imageConfig.format;
	auto width = imageConfig.extent.width;
	auto height = imageConfig.extent.height; 

	VkImageCreateInfo imageInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //don't care about the initial layout;
	imageInfo.usage = usage;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; //basically default;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

}


void FVkTexture::createImageView()
{   
	auto& device = deviceRef.lock()->vkDevice;

	auto& imageConfig = *(this->imageConfig);
	auto format = imageConfig.format;
	auto aspect = imageConfig.aspect; 

	VkImageViewCreateInfo viewInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	 
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}


void FVkTexture::createSampler() { 

	auto& physicalDevice = deviceRef.lock()->vkPhysicalDevice;
	auto& device = deviceRef.lock()->vkDevice; 

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &this->sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

}

void FVkTexture::createFBDepthTexture(uint32_t width, uint32_t height, VkFormat format)
{  
	auto& device = deviceRef.lock()->vkDevice;
	auto& heapManager = Global::vulkanRHI->heapManagerRef; 

	//hardcode for now
	this->imageConfig = {
		{width, height, 1},
		format,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT
	}; 
	 
	auto& imageConfig = this->imageConfig.value(); 
  
	createVkImage();
	this->imageMemory = heapManager->allocateImageMemory(image, imageConfig.memoryProperty);
	vkBindImageMemory(device, image, imageMemory, 0); 

	createImageView();   
}



void FVkTexture::generateMipmaps() {

	auto& physicalDevice = deviceRef.lock()->vkPhysicalDevice; 

	auto& imageConfig = *(this->imageConfig);
	auto imageFormat = imageConfig.format;
	auto texWidth = imageConfig.extent.width;
	auto texHeight = imageConfig.extent.height;

	auto& commandContextRef = Global::vulkanRHI->commandContextRef;



	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	} 
	
	VkCommandBuffer commandBuffer = commandContextRef->beginGraphicsTransientCmdBuffer();
	 
	VkImageMemoryBarrier barrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER }; 
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	//  i-1 level, blit and transit to shader_readonly,
	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

    //the last level
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	  
	
	commandContextRef->endGraphicsTransientCmdBuffer(commandBuffer);


}

 
void FVkTexture::createStorageImage(uint32_t width, uint32_t height) {

	auto& device = deviceRef.lock()->vkDevice;
	auto& heapManager = Global::vulkanRHI->heapManagerRef;
	auto& commandContextRef = Global::vulkanRHI->commandContextRef;

	//hardcode for now
	//both src and dst if we generate mipmaps ; then it's also transfer src;
	this->imageConfig = {
		{width, height, 1},
		VK_FORMAT_R8G8B8A8_UNORM,  //hardcode RGBA for now
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
	}; 

	 
	auto& imageConfig = this->imageConfig.value(); 

	//describe the transitions 
	FVkImageStageConfig initStage{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, 0 };
	FVkImageStageConfig computeStage{ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT };
	FVkImageStageConfig renderStage{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT };

	createVkImage();
	this->imageMemory = heapManager->allocateImageMemory(image, imageConfig.memoryProperty);
	vkBindImageMemory(device, image, imageMemory, 0);
	 

	createImageView();
	createSampler();



	auto commandBuffer = commandContextRef->beginComputeTransientCmdBuffer();
	InsertImageBarrier(commandBuffer, image, initStage, computeStage);
	commandContextRef->endComputeTransientCmdBuffer(commandBuffer);

}






 