#pragma once
#include "PCH.h"  
#include "Math/MMath.h"   

#include "Mesh.h"
#include "StaticMeshActor.h" 

namespace Mesh {
    //strip out the minimum to render a static mesh:
    struct FStaticMeshProxy {
        Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

        UStaticMesh* mesh;

        FMaterialProxy* material;

        InstanceData* instanceData;
        size_t instanceCount;
    };
}