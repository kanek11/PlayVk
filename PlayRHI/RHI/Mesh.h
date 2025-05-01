#pragma once

//vk depth is in range [0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "GVulkanRHI.h"
#include "Buffer.h"



//std140 layout
//you might want to group multiple UVs together?
struct alignas(16) StaticMeshVertex {
	alignas(16) glm::vec3 position;   // 12 bytes  
	alignas(16) glm::vec2 UV;         // 8 bytes  
	alignas(16) glm::vec3 normal;     
	alignas(16) glm::vec3 tangent;    
};


/*
* class StaticeMeshVertexMetaInfo
{
public:
	struct VertexMemberInfo
	{
		//const char* name;   // for debugging and reflection
		VkFormat format;
		uint32_t offset;
		uint32_t size;
		uint32_t padded_size;
	};

public:
	uint32_t stride;  //or stride
	std::array<VertexMemberInfo, 4> members;

	StaticeMeshVertexMetaInfo()
	{
		stride = sizeof(StaticMeshVertex);
		members[0] = { VK_FORMAT_R32G32B32_SFLOAT, offsetof(StaticMeshVertex, position), sizeof(glm::vec3), 16 };
		members[1] = { VK_FORMAT_R32G32_SFLOAT, offsetof(StaticMeshVertex, UV), sizeof(glm::vec2), 16 };
		members[2] = { VK_FORMAT_R32G32B32_SFLOAT, offsetof(StaticMeshVertex, normal), sizeof(glm::vec3) , 16 };
		members[3] = { VK_FORMAT_R32G32B32_SFLOAT, offsetof(StaticMeshVertex, tangent), sizeof(glm::vec3) ,  16 };
	}


};
*/




struct StaticMeshArrays
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
};

/*
* bridge to the vulkan pipeline
* potential for mesh optimizer
* instance in this engine is always in SSAO, so the inputRate here is always per-vertex
*/
class FVkStaticMeshResource
{
public:
	FVkStaticMeshResource(SharedPtr<FVkDevice> device,
		StaticMeshArrays& vertexData, std::vector<uint16_t>& indices)
		: deviceRef(device), vertexData(vertexData), indices(indices) {
		CreateVkResource();
	};

	void CreateVkResource();

	std::vector<StaticMeshVertex> ConsolidateVertexData();

	StaticMeshArrays& vertexData;
	std::vector<uint16_t>& indices;

	SharedPtr<FVkBuffer> vertexBuffer;   //todo£º can have multiple 
	SharedPtr<FVkBuffer> indexBuffer;

	//size of the vertex struct members
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;



private:
	SharedPtr<FVkDevice> deviceRef;

};


/*
 implementation-agnotic class
 * it won't assume how vertex data is organized for the GPU;
*/

class UStaticMesh
{

public:

	void CreateRHIResource()
	{
		StaticMeshArrays vertexData = { positions, UVs, normals, tangents };

		staticMeshResource = CreateShared<FVkStaticMeshResource>(
			Global::vulkanRHI->deviceRef,
			vertexData, indices);
	}


public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;

	std::vector<uint16_t> indices;



	//handle to the resource;
	SharedPtr<FVkStaticMeshResource> staticMeshResource;
};