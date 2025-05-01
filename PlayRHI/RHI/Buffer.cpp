
#include "Buffer.h"

FVkBuffer::~FVkBuffer()
{
}


FVkBuffer::FVkBuffer(SharedPtr<FVkDevice> device, EBufferUsage usage, VkDeviceSize size) :
	deviceRef(device)
{
	auto bufferInfo = bufferUsageMap[usage];
	this->config = { size, bufferInfo.bufferUsage, bufferInfo.memoryProperty };

	CreateVkBuffer();
	AllocateMemory(); 
	vkBindBufferMemory(device->vkDevice, buffer, memory, 0);

}

bool FVkBuffer::CreateVkBuffer()
{
	auto device = deviceRef->vkDevice;

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = config.size;
	bufferInfo.usage = config.usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	return true;
}


void FVkBuffer::AllocateMemory()
{
	auto device = deviceRef->vkDevice;
	auto heapManager = Global::vulkanRHI->heapManagerRef;

	memory = heapManager->AllocateBufferMemory(buffer, config.memoryProperty);
}
