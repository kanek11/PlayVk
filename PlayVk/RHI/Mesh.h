#pragma once


#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "GVulkanRHI.h"
#include "Buffer.h"

//todo£º make the data description like "schema"?


/*
* class StaticeMeshVertexMetaInfo
{
public:
	struct VertexInputMemberInfo
	{
		//const char* name;   // for debugging and reflection
		VkFormat format;
		uint32_t offset;
		uint32_t size;
		uint32_t padded_size;
	};

public:
	uint32_t stride;  //or stride
	std::array<VertexInputMemberInfo, 4> members;

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


//std140 layout
//you might want to group multiple UVs together?
struct alignas(16) StaticMeshVertex {
	alignas(16) glm::vec3 position;   // 12 bytes  
	alignas(16) glm::vec2 UV;         // 8 bytes  
	alignas(16) glm::vec3 normal;     
	alignas(16) glm::vec3 tangent;    
};

//trivial solution for now:
struct StaticMeshVertexDesc {
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();


};
 

struct StaticMeshArraysRef
{
	std::vector<glm::vec3>& positions;
	std::vector<glm::vec2>& UVs;
	std::vector<glm::vec3>& normals;
	std::vector<glm::vec3>& tangents;
};

/*
* bridge to the vulkan pipeline
* potential for mesh optimizer
* instance in this engine is always in SSAO, so the inputRate here is always per-vertex
*/
class FVkStaticMeshResource
{
public: 
	explicit FVkStaticMeshResource(WeakPtr<FVkDevice> _deviceRef,
		StaticMeshArraysRef vertexData, std::vector<uint32_t>& indices)
		: vertexDataRef(vertexData), indicesRef(indices), 
		deviceRef(_deviceRef)
	{ 
		createVkResource();
	};

	FVkStaticMeshResource(const FVkStaticMeshResource&) = delete;
	FVkStaticMeshResource& operator=(const FVkStaticMeshResource&) = delete;

	FVkStaticMeshResource(FVkStaticMeshResource&&) = delete;
	FVkStaticMeshResource& operator=(FVkStaticMeshResource&&) = delete;

	 
public:
	SharedPtr<FVkBuffer> vertexBuffer;   //todo£º can have multiple 
	SharedPtr<FVkBuffer> indexBuffer;

private:  
	void createVkResource();
	std::vector<StaticMeshVertex> ConsolidateVertexData();

	const StaticMeshArraysRef vertexDataRef;
	const std::vector<uint32_t>& indicesRef;

private:
	const WeakPtr<FVkDevice> deviceRef; 
};


/*
 implementation-agnotic class
 * it won't assume how vertex data is organized for the GPU;
*/

class UStaticMesh
{ 
public:
	~UStaticMesh() {
		std::cout << "UStaticMesh destroyed" << '\n';
	}
	UStaticMesh() = default; 

	void CreateRHIResource()
	{  
		staticMeshResource = CreateShared<FVkStaticMeshResource>(Global::vulkanRHI->deviceRef,
			StaticMeshArraysRef{ positions, UVs, normals, tangents }, 
			indices);
	} 

public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;

	std::vector<uint32_t> indices;  

	//handle to the resource;
	SharedPtr<FVkStaticMeshResource> staticMeshResource;
};


//inline void MergeStaticMeshes(UStaticMesh& origin, UStaticMesh& src)
//{
//	origin.positions.insert(origin.positions.end(), src.positions.begin(), src.positions.end());
//	origin.UVs.insert(origin.UVs.end(), src.UVs.begin(), src.UVs.end());
//	origin.normals.insert(origin.normals.end(), src.normals.begin(), src.normals.end());
//	origin.tangents.insert(origin.tangents.end(), src.tangents.begin(), src.tangents.end());
//
//	//update indices
//	uint32_t offset = origin.indices.size();
//	for (auto& index : src.indices)
//	{
//		origin.indices.push_back(index + offset);
//	}
//}