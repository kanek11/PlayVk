
#include "Mesh.h"

std::vector<VkVertexInputAttributeDescription> StaticMeshVertexDesc::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(StaticMeshVertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(StaticMeshVertex, UV);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(StaticMeshVertex, normal);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(StaticMeshVertex, tangent);

	return attributeDescriptions;

}

std::vector<VkVertexInputBindingDescription> StaticMeshVertexDesc::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(StaticMeshVertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}


/*
* simple one buffer for now
*/
void FVkStaticMeshResource::createVkResource()
{   

	auto verticesSize = sizeof(StaticMeshVertex) * vertexDataRef.positions.size();
	auto indicesSize = sizeof(indicesRef[0]) * indicesRef.size();
	 

	this->vertexBuffer = CreateShared<FVkBuffer>(deviceRef ,FVkBufferDesc{ eVERTEX_BUFFER,  static_cast<VkDeviceSize>(verticesSize) });
	this->indexBuffer = CreateShared<FVkBuffer>(deviceRef, FVkBufferDesc{ eINDEX_BUFFER,  static_cast<VkDeviceSize>(indicesSize) });

	std::vector<StaticMeshVertex> verticeData = ConsolidateVertexData();


	auto& heapManager = Global::vulkanRHI->heapManagerRef;
	heapManager->updateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer->buffer, vertexBuffer->memory, verticesSize, verticeData.data());
	heapManager->updateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer->buffer, indexBuffer->memory, indicesSize, indicesRef.data());
	 

}


//todo: check valid length, 
std::vector<StaticMeshVertex> FVkStaticMeshResource::ConsolidateVertexData()
{ 
	std::vector<StaticMeshVertex> verticeData;  

	for (size_t i = 0; i < vertexDataRef.positions.size(); i++)
	{
		StaticMeshVertex vertex{};
		vertex.position = vertexDataRef.positions[i];

		if (i < vertexDataRef.UVs.size())
			vertex.UV = vertexDataRef.UVs[i];
		if (i < vertexDataRef.normals.size())
			vertex.normal = vertexDataRef.normals[i];
		if (i < vertexDataRef.tangents.size())
			vertex.tangent = vertexDataRef.tangents[i];

		verticeData.push_back(vertex);
	}

	return verticeData;

}


