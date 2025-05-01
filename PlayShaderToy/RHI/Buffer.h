#pragma once 
#include <vulkan/vulkan.h>

#include "GVulkanRHI.h"
#include "Base.h"

#include <unordered_map>
#include <optional>
/*

* all resource-related functionalities.

*/
//todo :  




/*
* used to create buffer memory
* open for extension
*/
//struct FAllocMemoryCreateInfo
//{
//	VkDeviceSize bufferSize;
//
//};

struct FVkVertexBufferCreateInfo
{
	VkDeviceSize bufferSize;
};

struct FVkIndexBufferCreateInfo
{
	VkDeviceSize bufferSize;
};



enum class EBufferUsage
{
	eVERTEX_BUFFER,
	eINDEX_BUFFER,
	eUNIFORM_BUFFER,
	eSTORAGE_BUFFER,
	eSTAGING_BUFFER,
	eINDIRECT_BUFFER,
};

using enum EBufferUsage;

struct FVkBufferUsageConfig
{
	VkBufferUsageFlags bufferUsage;
	VkMemoryPropertyFlags memoryProperty;
};


/*
* todo: make this configurable by the user
*
* allocator can query the info by preset usage
*
* host visible = on the cpu-side, can be directly updated by vkMapMemory
*
* host visible + coherent = the sync is handled automatically
*
* device local = must be copied from, a host visible buffer(temp staging);
*
*/
inline std::unordered_map<EBufferUsage, FVkBufferUsageConfig> bufferUsageMap = {
	// Vertex buffer usage
	{
		eVERTEX_BUFFER,
		{
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		}
	},

	// Index buffer usage
	{
		eINDEX_BUFFER,
		{
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		}
	},

	// Uniform buffer usage
	{
		eUNIFORM_BUFFER,
		{
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		}
	},

	 { eSTORAGE_BUFFER,
		{ VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		}
	},

	// Staging buffer usage
	{
		eSTAGING_BUFFER,
		{
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		}
	},

	 { eINDIRECT_BUFFER,
		{ VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		}
	},
};





//simple memory for now,  
//it should be a memory pool that containers multiple logical, engine-level objects;

class FVkBuffer
{
public:
	struct FVkBufferConfig
	{
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memoryProperty;
	};


public:
	FVkBuffer(SharedPtr<FVkDevice>, EBufferUsage usage, VkDeviceSize size);
	~FVkBuffer();

	bool createVkBuffer();
	void allocateMemory();

public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;

	FVkBufferConfig bufferConfig;

private:
	const SharedPtr<FVkDevice> deviceRef;
};


/*
* RHI-level factory
*/
//SharedPtr<FVkBuffer> CreateVkVertexBuffer(const FVkVertexBufferCreateInfo& info);
//SharedPtr<FVkBuffer> CreateVkIndexBuffer(const FVkIndexBufferCreateInfo& info);
