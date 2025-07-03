#pragma once
#include "PCH.h"
#include "Base.h"

#include "Resource.h"

#include "Math/Vector.h"

//abstract of mesh; only consider static mesh for now;


using namespace DirectX;

//using FLOAT3 = XMFLOAT3;
//using FLOAT2 = XMFLOAT2;
//using FLOAT4 = XMFLOAT4;

using FLOAT3 = MMath::FLOAT3;
using FLOAT2 = MMath::FLOAT2;
using FLOAT4 = MMath::FLOAT4;

using INDEX_FORMAT = uint16_t;


struct FD3D12InputDesc {
	std::string semanticName;
	uint32_t semanticIndex = 0;
	DXGI_FORMAT format;
	uint32_t alignedByteOffset;
	uint32_t byteSize;
	uint32_t paddedSize;
};


struct StaticMeshVertex
{
	FLOAT3 position;
	FLOAT3 normal;
	FLOAT3 tangent;
	FLOAT4 color;
	FLOAT2 texCoord0;
};


struct StaticMeshInputDesc
{
	static const std::vector<FD3D12InputDesc>& GetInputDescs()
	{
		static const std::vector<FD3D12InputDesc> table =
		{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, position), sizeof(FLOAT3), 16 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, normal), sizeof(FLOAT3), 16 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, tangent), sizeof(FLOAT3), 16 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(StaticMeshVertex, color), sizeof(FLOAT4), 16 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(StaticMeshVertex, texCoord0), sizeof(FLOAT2), 16 }
		};

		return table;
	}
};



struct StaticMeshData
{
	std::vector<FLOAT3> positions;
	std::vector<FLOAT2> UVs;
	std::vector<FLOAT3> normals;
	std::vector<FLOAT3> tangents;
	std::vector<FLOAT4> colors;

	std::vector<INDEX_FORMAT> indices; // Use 16-bit indices for smaller memory footprint 

	std::vector<StaticMeshVertex> vertices;

	std::vector<StaticMeshVertex> ConsolidateVertexData() const
	{
		std::vector<StaticMeshVertex> consolidatedVertices;
		consolidatedVertices.reserve(positions.size());

		for (size_t i = 0; i < positions.size(); ++i) {
			StaticMeshVertex vertex;
			vertex.position = positions[i];
			if (i < UVs.size()) vertex.texCoord0 = UVs[i];
			if (i < normals.size()) vertex.normal = normals[i];
			if (i < tangents.size()) vertex.tangent = tangents[i];
			if (i < colors.size()) vertex.color = colors[i];
			consolidatedVertices.push_back(vertex);
		}
		return consolidatedVertices;  //the copy behavior is left to RVO
	}



	D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};


class FD3D12MeshResource {
public:
	virtual ~FD3D12MeshResource() = default;
	FD3D12MeshResource(ID3D12Device* device, const StaticMeshData& meshData) :
		m_device(device),
		m_meshData(meshData)
	{
		CreateResources();
	}

	void CreateResources();


public:
	const StaticMeshData& m_meshData; // Reference to the mesh data
	SharedPtr<FD3D12Buffer> m_vertexBuffer{ nullptr };
	SharedPtr<FD3D12Buffer> m_indexBuffer{ nullptr };

private:
	ID3D12Device* m_device = nullptr; // Device for resource creation
};



class UStaticMesh {
public:
	virtual ~UStaticMesh() = default;
	UStaticMesh() = default;

public:
	virtual void CreateMeshData() = 0;

public:
	void CreateGPUResource(ID3D12Device* device);

public:
	//getters:
	virtual const std::vector<StaticMeshVertex>& GetVertices() const {
		return m_meshData.vertices;
	}
	virtual const std::vector<INDEX_FORMAT>& GetIndices() const {
		return m_meshData.indices;
	}

	size_t GetVertexCount() const
	{
		return static_cast<int>(m_meshData.vertices.size());
	}

	size_t GetIndexCount() const
	{
		return static_cast<int>(m_meshData.indices.size());
	}

	SharedPtr<FD3D12Buffer> GetVertexBuffer() const
	{
		return m_GPUResource ? m_GPUResource->m_vertexBuffer : nullptr;
	}

	SharedPtr<FD3D12Buffer> GetIndexBuffer() const
	{
		return m_GPUResource ? m_GPUResource->m_indexBuffer : nullptr;
	}

	D3D_PRIMITIVE_TOPOLOGY GetTopology() const
	{
		return m_meshData.topology;
	}


public:
	StaticMeshData m_meshData;

	SharedPtr<FD3D12MeshResource> m_GPUResource;
};


class CubeMesh : public UStaticMesh {
public:
	virtual ~CubeMesh() = default;
	CubeMesh();

	void CreateMeshData() override;
};


class PlaneMesh : public UStaticMesh {
public:
	virtual ~PlaneMesh() = default;
	PlaneMesh();
	void CreateMeshData() override;
};


class SphereMesh : public UStaticMesh {
public:
	virtual ~SphereMesh() = default;
	SphereMesh();
	void CreateMeshData() override;
};