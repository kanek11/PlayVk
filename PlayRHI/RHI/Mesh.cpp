
#include "Mesh.h"


/*
* simple one buffer for now
*/
void FVkStaticMeshResource::CreateVkResource()
{
	//
	attributeDescriptions.resize(4);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
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


	//
	bindingDescriptions.resize(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(StaticMeshVertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


	auto verticesSize = sizeof(StaticMeshVertex) * vertexData.positions.size();
	auto indicesSize = sizeof(indices[0]) * indices.size();

	this->vertexBuffer = CreateShared<FVkBuffer>(deviceRef, VERTEX_BUFFER, static_cast<VkDeviceSize>(verticesSize));
	this->indexBuffer = CreateShared<FVkBuffer>(deviceRef, INDEX_BUFFER, static_cast<VkDeviceSize>(indicesSize)); 

	std::vector<StaticMeshVertex> verticeData = ConsolidateVertexData();


	auto heapManager = Global::vulkanRHI->heapManagerRef;
	heapManager->UpdateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer->buffer, vertexBuffer->memory, verticesSize, verticeData.data());
	heapManager->UpdateBufferMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer->buffer, indexBuffer->memory, indicesSize, indices.data());



}



std::vector<StaticMeshVertex> FVkStaticMeshResource::ConsolidateVertexData()
{

	std::vector<StaticMeshVertex> verticeData;

	for (size_t i = 0; i < vertexData.positions.size(); i++)
	{
		StaticMeshVertex vertex{};
		vertex.position = vertexData.positions[i];

		if (i < vertexData.UVs.size())
			vertex.UV = vertexData.UVs[i];
		if (i < vertexData.normals.size())
			vertex.normal = vertexData.normals[i];
		if (i < vertexData.tangents.size())
			vertex.tangent = vertexData.tangents[i];

		verticeData.push_back(vertex);
	}

	return verticeData;

}