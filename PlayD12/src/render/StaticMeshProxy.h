#pragma once
#include "PCH.h"  
#include "Math/MMath.h"   

#include "Mesh.h"
#include "Material.h"

//namespace Mesh {

    struct InstanceData
    {
        MMath::Float3 offset;
    };

    template <>
    struct VertexLayoutTraits<InstanceData> {
        static constexpr bool is_specialized = true;
        static constexpr std::array<VertexAttribute, 1> attributes = {
            VertexAttribute{ "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(InstanceData, offset) }
        };
    };

    //strip out the minimum to render a static mesh:
    struct FStaticMeshProxy {
        Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

        UStaticMesh* mesh;

        UMaterial* material;

        InstanceData* instanceData;
        size_t instanceCount;
    };




//}