
#include "Buffer.h"

FVkBuffer::~FVkBuffer()
{
}


FVkBuffer::FVkBuffer(SharedPtr<FVkDevice> device, EBufferUsage usage, VkDeviceSize size) :
	deviceRef(device)
{
	auto bufferInfo = bufferUsageMap[usage];
	this->bufferConfig = { size, bufferInfo.bufferUsage, bufferInfo.memoryProperty };

	createVkBuffer();
	allocateMemory(); 
	vkBindBufferMemory(device->vkDevice, buffer, memory, 0);

}

bool FVkBuffer::createVkBuffer()
{
	auto device = deviceRef->vkDevice;

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = bufferConfig.size;
	bufferInfo.usage = bufferConfig.usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	return true;
}


void FVkBuffer::allocateMemory()
{
	auto device = deviceRef->vkDevice;
	auto heapManager = Global::vulkanRHI->heapManagerRef;

	memory = heapManager->allocateBufferMemory(buffer, bufferConfig.memoryProperty);
}
