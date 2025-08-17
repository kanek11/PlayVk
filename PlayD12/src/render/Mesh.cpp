#include "PCH.h"
#include "Mesh.h" 


void FD3D12MeshResource::CreateResources()
{

    //m_vertexBuffer = CreateShared<FD3D12Buffer>(m_device, FBufferDesc{
    //    m_meshData.vertices.size() * sizeof(StaticMeshVertex),
    //    DXGI_FORMAT_UNKNOWN, // Not used for vertex buffers
    //    sizeof(StaticMeshVertex),
    //    EBufferUsage::Upload | EBufferUsage::Vertex
    //    });


	m_vertexBuffer = Buffer::CreateStructured<StaticMeshVertex>(m_device, Buffer::FStructuredDesc{
		.count = m_meshData.vertices.size(),
		.Usage = EBufferUsage::Upload | EBufferUsage::Vertex
		});

    m_vertexBufferView = Buffer::CreateBVStructured<StaticMeshVertex>(
        m_vertexBuffer 
    );

    m_vertexBuffer->UploadDataStructured(m_meshData.vertices.data(), m_meshData.vertices.size());


    // Create index buffer
    //m_indexBuffer = CreateShared<FD3D12Buffer>(m_device, FBufferDesc{
    //    m_meshData.indices.size() * sizeof(INDEX_FORMAT),
    //    DXGI_FORMAT_R16_UINT, // 16-bit indices
    //    sizeof(INDEX_FORMAT),
    //    EBufferUsage::Upload | EBufferUsage::Index
    //    });

	m_indexBuffer = Buffer::CreateStructured<INDEX_FORMAT>(m_device, Buffer::FStructuredDesc{
		.count = m_meshData.indices.size(),
		.Usage = EBufferUsage::Upload | EBufferUsage::Index
		});

	m_indexBufferView = Buffer::CreateBVStructured<INDEX_FORMAT>(
		m_indexBuffer, INDEX_FORMAT_DX
	);

    m_indexBuffer->UploadDataStructured(m_meshData.indices.data(), m_meshData.indices.size());

}

void UStaticMesh::CreateGPUResource(ID3D12Device* device)
{
    m_GPUResource = CreateShared<FD3D12MeshResource>(device, m_meshData);

    uploaded = true; // Mark as uploaded  
}


CubeMesh::CubeMesh()
{
    CreateMeshData();
    std::cout << "CubeMesh created with " << m_meshData.vertices.size() << " vertices and "
        << m_meshData.indices.size() << " indices." << std::endl;

}

void CubeMesh::CreateMeshData()
{
    std::vector<Float3> positions;
    std::vector<Float2> uvs;
    std::vector<Float3> normals;
    std::vector<Float3> tangents;
    std::vector<Float4> colors;
    std::vector<INDEX_FORMAT> indices;

    int reserveSize = 36;
    positions.reserve(reserveSize);
    uvs.reserve(reserveSize);
    normals.reserve(reserveSize);
    tangents.reserve(reserveSize);
    colors.reserve(reserveSize);
    indices.reserve(reserveSize);


    // Define face data: 6 sides
    struct Face {
        Float3 normal;
        Float3 tangent;
        Float3 vertices[4];
    };

    const Float2 uvTemplate[4] = {
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}
    };

    const Float4 faceColor[6] = {
        {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1},
        {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
    };

    const Face faces[6] = {
        // Front (+Z)
        { { 0, 0, 1 }, { 1, 0, 0 }, {
            {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} } },
            // Back (-Z)
            { { 0, 0, -1 }, { -1, 0, 0 }, {
                {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} } },
                // Left (-X)
                { {-1, 0, 0 }, { 0, 0, -1 }, {
                    {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1} } },
                    // Right (+X)
                    { {1, 0, 0 }, { 0, 0, 1 }, {
                        {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1} } },
                        // Top (+Y)
                        { {0, 1, 0 }, {1, 0, 0}, {
                            {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1} } },
                            // Bottom (-Y)
                            { {0, -1, 0 }, {1, 0, 0}, {
                                {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1} } },
    };

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx)
    {
        const Face& face = faces[faceIdx];

        int baseIndex = static_cast<int>(positions.size());

        int triTable[6] = { 0, 2, 1, 0, 3, 2 };
        for (int i = 0; i < 6; ++i)
        {
            int vi = triTable[i];
            positions.push_back(face.vertices[vi]);
            uvs.push_back(uvTemplate[vi]);
            normals.push_back(face.normal);
            tangents.push_back(face.tangent);
            colors.push_back(faceColor[faceIdx]);
            indices.push_back(baseIndex + i);
        }
    }


    m_meshData.positions = positions;
    m_meshData.UVs = uvs;
    m_meshData.normals = normals;
    m_meshData.tangents = tangents;
    m_meshData.colors = colors;
    m_meshData.indices = indices;

    // Set topology
    m_meshData.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // triangle list

    m_meshData.vertices = m_meshData.ConsolidateVertexData();
}



PlaneMesh::PlaneMesh(uint32_t subdivisionX, uint32_t subdivisionZ) :
    subdivisionX(subdivisionX), subdivisionZ(subdivisionZ)
{
    CreateMeshData();
    std::cout << "PlaneMesh created with " << m_meshData.vertices.size() << " vertices and "
        << m_meshData.indices.size() << " indices." << std::endl;
}


void PlaneMesh::CreateMeshData()
{
    // Customizable inputs
    uint32_t subdivisionX = this->subdivisionX;
    uint32_t subdivisionZ = this->subdivisionZ;

    float tileSizeX = 4.0f; // Size of each checker tile in world units (along X)
    float tileSizeZ = 4.0f;

    float quadSizeX = 1.0f; // Each quad is 1x1 unit in world space
    float quadSizeZ = 1.0f;

    float physicalSizeX = quadSizeX * subdivisionX;
    float physicalSizeZ = quadSizeZ * subdivisionZ;

    uint32_t numX = subdivisionX + 1;
    uint32_t numZ = subdivisionZ + 1;

    uint32_t numVertices = numX * numZ;
    uint32_t numTriangles = subdivisionX * subdivisionZ * 2;

    std::vector<Float3> _positions(numVertices);
    std::vector<Float2> _UVs(numVertices);

    Float3 offset = { -physicalSizeX / 2.0f, 0.0f, -physicalSizeZ / 2.0f };

    // Step size in world units
    float stepX = physicalSizeX / (float)(numX - 1);
    float stepZ = physicalSizeZ / (float)(numZ - 1);

    // Generate vertices and UVs
    for (uint32_t i = 0; i < numZ; i++)
    {
        for (uint32_t j = 0; j < numX; j++)
        {
            float worldX = j * stepX + offset.x();
            float worldZ = i * stepZ + offset.z();

            uint32_t index = j + i * numX;

            _positions[index] = Float3{ worldX, 0.0f, worldZ };
            _UVs[index] = Float2{ worldX / tileSizeX, worldZ / tileSizeZ };
        }
    }

    // Generate triangle indices
    std::vector<INDEX_FORMAT> _indices(numTriangles * 3);

    for (uint32_t i = 0; i < subdivisionZ; i++)
    {
        for (uint32_t j = 0; j < subdivisionX; j++)
        {
            uint32_t baseIndex = (j + i * subdivisionX) * 6;

            uint32_t v0 = j + i * numX;
            uint32_t v1 = j + (i + 1) * numX;
            uint32_t v2 = (j + 1) + (i + 1) * numX;
            uint32_t v3 = (j + 1) + i * numX;

            _indices[baseIndex + 0] = v0;
            _indices[baseIndex + 1] = v1;
            _indices[baseIndex + 2] = v2;

            _indices[baseIndex + 3] = v0;
            _indices[baseIndex + 4] = v2;
            _indices[baseIndex + 5] = v3;
        }
    }

    // Fill remaining mesh data
    Float3 normal = Float3{ 0.0f, 1.0f, 0.0f };
    Float3 tangent = Float3{ 1.0f, 0.0f, 0.0f };

    m_meshData.positions = _positions;
    m_meshData.UVs = _UVs;
    m_meshData.normals = std::vector<Float3>(_positions.size(), normal);
    m_meshData.tangents = std::vector<Float3>(_positions.size(), tangent);
    m_meshData.colors = std::vector<Float4>(_positions.size(), Float4{ 1.0f, 1.0f, 1.0f, 1.0f });
    m_meshData.indices = _indices;

    m_meshData.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_meshData.vertices = m_meshData.ConsolidateVertexData();
}




/*
void PlaneMesh::CreateMeshData()
{
    uint32_t subdivision = 2; // 2 means 1x1 grid, 3 means 2x2 grid, etc.

    // divide by 2 means 3x3 grid
    uint32_t numX = subdivision + 1;
    uint32_t numZ = subdivision + 1;

    auto numVertices = numX * numZ;
    auto numTriangles = (numX - 1) * (numZ - 1) * 2;

    std::vector<Float3> _positions(numVertices);
    std::vector<Float2> _UVs(numVertices);

    Float3 offset = { -0.5f, 0.0f, -0.5f }; // center at origin

    // j means width/x , i means height/z
    // loop row by row
    for (uint32_t i = 0; i < numZ; i++)
    {
        for (uint32_t j = 0; j < numX; j++)
        {
            _positions[j + i * numX] = Float3{ (float)j / (numX - 1) + offset.x(), 0.0f + offset.y(), (float)i / (numZ - 1) + offset.z() };
            _UVs[j + i * numX] = Float2{ (float)j / (numX - 1), (float)i / (numZ - 1) };
        }
    }

    // 0 1 in  0 2 3 , 0 3 1
    // 2 3
    std::vector<INDEX_FORMAT> _indices(numTriangles * 3);

    for (uint32_t i = 0; i < numZ - 1; i++)
    {
        for (uint32_t j = 0; j < numX - 1; j++)
        {
            _indices[(j + i * (numX - 1)) * 6 + 0] = j + i * numX;
            _indices[(j + i * (numX - 1)) * 6 + 1] = j + (i + 1) * numX;
            _indices[(j + i * (numX - 1)) * 6 + 2] = (j + 1) + (i + 1) * numX;

            _indices[(j + i * (numX - 1)) * 6 + 3] = j + i * numX;
            _indices[(j + i * (numX - 1)) * 6 + 4] = (j + 1) + (i + 1) * numX;
            _indices[(j + i * (numX - 1)) * 6 + 5] = j + 1 + i * numX;
        }
    }


    Float3 normal = Float3{ 0.0, 1.0, 0.0 }; // y axis  world up
    Float3 tangent = Float3{ 1.0, 0.0, 0.0 }; // x axis


    m_meshData.positions = _positions;
    m_meshData.UVs = _UVs;
    m_meshData.normals = std::vector<Float3>(_positions.size(), normal);
    m_meshData.tangents = std::vector<Float3>(_positions.size(), tangent);
    m_meshData.colors = std::vector<Float4>(_positions.size(), Float4{ 1.0f, 1.0f, 1.0f, 1.0f }); // white color
    m_meshData.indices = _indices;

    //topo:
    m_meshData.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // triangle list

    m_meshData.vertices = m_meshData.ConsolidateVertexData();

}


*/
SphereMesh::SphereMesh()
{
    CreateMeshData();
    std::cout << "SphereMesh created with " << m_meshData.vertices.size() << " vertices and "
        << m_meshData.indices.size() << " indices." << std::endl;

}

void SphereMesh::CreateMeshData()
{
    uint32_t subdivision = 20;
    const float    PI = 3.14159265359f;

    std::vector<Float3> _positions;
    std::vector<Float3> _normals;
    std::vector<Float2> _UVs;
    std::vector<Float3> _Tangents;

    std::vector<INDEX_FORMAT> _indices;

    const uint32_t X_SEGMENTS = subdivision;
    const uint32_t Y_SEGMENTS = subdivision;

    // x = azimuth y= elevation
    for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
    {
        for (uint32_t y = 0; y <= Y_SEGMENTS; ++y)
        {
            // normalized
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPhi = xSegment * 2.0f * PI;
            float yTheta = ySegment * PI;

            // y being up
            float yPos = std::cos(yTheta);
            float xPos = std::cos(xPhi) * std::sin(yTheta);
            float zPos = std::sin(xPhi) * std::sin(yTheta);

            _positions.push_back(Float3{ xPos, yPos, zPos });
            _normals.push_back(Float3{ xPos, yPos, zPos });
            _UVs.push_back(Float2{ xSegment, ySegment });

            // tangent ,always on xz plane
            // as derivative to normal , dN/dphi = (-sin(phi), 0, cos(phi)) * sin(theta)

            // note at poles, sintheta = 0, tangent is undefined;
            Float3 _tangent;
            if (yTheta == 0)
            {
                _tangent = Float3{ 1.0f, 0.0f, 0.0f };
            }
            else if (yTheta == PI)
            {
                _tangent = Float3{ -1.0f, 0.0f, 0.0f };
            }
            else
            {
                Float3 Tangent = Float3{ -std::sin(xPhi) * std::sin(yTheta),
                    0,
                    std::cos(xPhi) * std::sin(yTheta) };

                //normalize:  
                _tangent = MMath::Normalize(Tangent);

            }

            _Tangents.push_back(_tangent);
        }
    }

    // generate indices ;
    //  draw "strip" mode ; +2 indices for each new triangle

    for (uint32_t y = 0; y < Y_SEGMENTS; ++y)
    {
        for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
        {
            _indices.push_back(y * (X_SEGMENTS + 1) + x);
            _indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        }
    }

    // generate mesh 
    m_meshData.positions = _positions;
    m_meshData.normals = _normals;
    m_meshData.UVs = _UVs;
    m_meshData.tangents = _Tangents;
    m_meshData.indices = _indices;
    m_meshData.colors = std::vector<Float4>(_positions.size(), Float4{ 1.0f, 1.0f, 1.0f, 1.0f }); // white color

    m_meshData.vertices = m_meshData.ConsolidateVertexData();

    //strip:
    m_meshData.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}