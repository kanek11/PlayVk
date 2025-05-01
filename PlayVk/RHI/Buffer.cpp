
#include "Buffer.h"




FVkBuffer::~FVkBuffer()
{
	if (!deviceRef.expired())
	{
		auto& device = deviceRef.lock()->vkDevice;
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
        
		buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
		std::cout << "Buffer destroyed" << '\n';
	}
	else {
		std::cerr << "error: destroy buffer after device!" << '\n';
	} 
	
}


FVkBuffer::FVkBuffer(WeakPtr<FVkDevice> device, FVkBufferDesc desc) :
	deviceRef(device)
{ 
	auto& usageMap = bufferUsageMap[desc.usage]; 
	this->bufferConfig = { desc.size, usageMap.bufferUsage, usageMap.memoryProperty, desc.format };

	if (auto resultOpt = createVkBuffer(); resultOpt.has_value()) {
		this->buffer = *resultOpt;
	}
	else {
		throw std::runtime_error("Failed to create buffer"); 
	} 

	allocateMemory(); 
	vkBindBufferMemory(device.lock()->vkDevice, buffer, memory, 0); 

}

std::optional<VkBuffer> FVkBuffer::createVkBuffer() const noexcept
{
	auto& device = deviceRef.lock()->vkDevice; 
	auto& config = *(this->bufferConfig);

	VkBufferCreateInfo bufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = config.size;
	bufferInfo.usage = config.usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer _buffer;
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) { 
		return std::nullopt;
	}
	else { 
		return _buffer;
	} 
}


void FVkBuffer::allocateMemory()
{ 
	auto& heapManager = Global::vulkanRHI->heapManagerRef; 
	auto& bufferConfig = *(this->bufferConfig);

	this->memory = heapManager->allocateBufferMemory(buffer, bufferConfig.memoryProperty);
}
