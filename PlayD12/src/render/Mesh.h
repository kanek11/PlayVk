#pragma once
#include "PCH.h"
#include "Base.h"

#include "Resource.h"

#include "Math/MMath.h"

//abstract of mesh; only consider static mesh for now;


//using Float3 = XMFLOAT3;
//using Float2 = XMFLOAT2;
//using Float4 = XMFLOAT4; 


using INDEX_FORMAT = uint16_t;




//the desc of a vertex attribute 
struct VertexAttribute {
	const char* semanticName;
	UINT semanticIndex;
	DXGI_FORMAT format;
	size_t offset;
};

// VertexLayoutTraits 
template<typename T>
struct VertexLayoutTraits {
	static constexpr bool is_specialized = false;
	static constexpr std::array<VertexAttribute, 0> attributes = {};
};

class InputLayoutBuilder {
public:
	template<typename Vertex>
	static std::vector<D3D12_INPUT_ELEMENT_DESC> Build(
		UINT slot = 0,
		D3D12_INPUT_CLASSIFICATION classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		UINT instanceStepRate = 0
	) {
		std::vector<D3D12_INPUT_ELEMENT_DESC> descs;
		const auto& attrs = VertexLayoutTraits<Vertex>::attributes;
		static_assert(VertexLayoutTraits<Vertex>::is_specialized, "VertexLayoutTraits must be specialized");


		for (const auto& attr : attrs) {
			D3D12_INPUT_ELEMENT_DESC d = {};
			d.SemanticName = attr.semanticName;
			d.SemanticIndex = attr.semanticIndex;
			d.Format = attr.format;
			d.InputSlot = slot;
			d.AlignedByteOffset = static_cast<UINT>(attr.offset);
			d.InputSlotClass = classification;
			d.InstanceDataStepRate = instanceStepRate;
			descs.push_back(d);
		}
		return descs;
	}
};




struct StaticMeshVertex
{
	Float3 position;
	Float3 normal;
	Float3 tangent;
	float tan_sign{};
	Float2 texCoord0;
	Float4 color;
};


template<>
struct VertexLayoutTraits<StaticMeshVertex> {
	static constexpr bool is_specialized = true;
	static constexpr auto attributes = std::to_array<VertexAttribute>({
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, position) },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, normal)   },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshVertex, tangent)  },
		{ "TANGENT_SIGN", 0, DXGI_FORMAT_R32_FLOAT,       offsetof(StaticMeshVertex, tan_sign) },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    offsetof(StaticMeshVertex, texCoord0)},
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(StaticMeshVertex, color) },
		});
};





struct StaticMeshData
{
	std::vector<Float3> positions;
	std::vector<Float2> UVs;
	std::vector<Float3> normals;
	std::vector<Float3> tangents;
	std::vector<Float4> colors;

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
	PlaneMesh(uint32_t subdivisionX, uint32_t subdivisionZ);
	void CreateMeshData() override;

	uint32_t subdivisionX, subdivisionZ;
};


class SphereMesh : public UStaticMesh {
public:
	virtual ~SphereMesh() = default;
	SphereMesh();
	void CreateMeshData() override;
};