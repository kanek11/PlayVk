#pragma once

#include "PCH.h" 

#include "D12Helper.h"

#include "Math/MMath.h"  

#include "RenderPass.h" 
#include "pipeline.h"   

#include "Resource.h" 
#include "Shader.h" 

#include "Text.h"  

#include "Mesh.h"


namespace Passes {

    inline RenderPassDesc ForwardPassDesc = {
     .passTag = "Forward",
    .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
    .depthFormat = DXGI_FORMAT_D32_FLOAT,
    .enableDepth = true,
    .enableBlend = false,
    .cullMode = D3D12_CULL_MODE_NONE,
    };

}

namespace Materials {

    inline MaterialDesc LitMaterialDesc = {
    .shaderTag = "Lit", 
    .enableAlphaBlend = false,
    .doubleSided = false,
    .depthWrite = true,
    };
}


namespace Lit
{ 
    struct InstanceData
    {
        MMath::Float3 offset;
    };  

    struct SceneCB
    {
        Float4x4  pvMatrix;
        //padding:
        float padding[48]; // 64 bytes total
    };

    struct ObjectCB
    {
        Float4x4 modelMatrix; // 64 bytes 
        float padding[48]; // 64 bytes total
    };




    struct GPUResources {
        ComPtr<ID3D12PipelineState> PSO;
        SharedPtr<FD3D12ShaderPermutation> shader;

        SharedPtr<FD3D12Buffer> sceneCB;
    };


    struct DrawCmd { 
         
        //UStaticMesh* mesh;  
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_INDEX_BUFFER_VIEW ibv; 
        uint32_t heapOffset = 0;  

        //Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();
        //SharedPtr<FD3D12Buffer> objectCB;
		D3D12_GPU_VIRTUAL_ADDRESS objectCBAddr; 

        //std::vector<InstanceData> instanceData;
        //SharedPtr<FD3D12Buffer> instanceBuffer;

        D3D12_VERTEX_BUFFER_VIEW instanceVBV;
        uint32_t instanceCount = 0;


        uint32_t heapOffset = 0;  

    };
    
    struct RenderData {  
        std::vector<DrawCmd> cmds;
    };





}
