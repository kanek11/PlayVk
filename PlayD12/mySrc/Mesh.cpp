#include "Mesh.h" 
CubeMesh::CubeMesh()
{
	CreateMeshData();

std::cout << "CubeMesh created with " << m_meshData.vertices.size() << " vertices and "
<< m_meshData.indices.size() << " indices." << std::endl;
}

void CubeMesh::CreateMeshData()
{
	std::vector<FLOAT3> positions;  
	std::vector<FLOAT2> uvs; 
	std::vector<FLOAT3> normals;  
	std::vector<FLOAT3> tangents;  
    std::vector<FLOAT4> colors; 
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
        FLOAT3 normal;
        FLOAT3 tangent;
        FLOAT3 vertices[4];
    };

    const FLOAT2 uvTemplate[4] = {
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}
    };

    const FLOAT4 faceColor[6] = {
        {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1},
        {1, 1, 0, 1}, {0, 1, 1, 1}, {1, 0, 1, 1}
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

	m_meshData.vertices = m_meshData.ConsolidateVertexData();
}
