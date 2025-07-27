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
#include "StaticMeshActor.h"

#include "ShaderParameters.h"
 

struct RendererContext;

namespace Passes {

    inline RenderPassDesc ForwardPassDesc = {
     .passTag = "Forward",
    .colorFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
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
 

//strip out the minimum to render a static mesh:
struct FStaticMeshProxy {
    Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

    UStaticMesh* mesh;

    FMaterialProxy* material;


    InstanceData* instanceData;
    size_t instanceCount;
};
     

namespace Mesh{
     

    struct GPUResources {
        ComPtr<ID3D12PipelineState> PSO;
        SharedPtr<FD3D12ShaderPermutation> shader;

        SharedPtr<FRingBufferAllocator<ObjectCB>> objCBAllocator;

        SharedPtr<FRingBufferAllocator<InstanceData>> instanceBufferAllocator;
        std::optional <uint32_t> baseHeapOffset = 0;
    };
     

    struct DrawCmd {

        //UStaticMesh* mesh;  
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_PRIMITIVE_TOPOLOGY topology;
        D3D12_INDEX_BUFFER_VIEW ibv;
        size_t indexCount; 

        //Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();
        //SharedPtr<FD3D12Buffer> objectCB;
        //D3D12_GPU_VIRTUAL_ADDRESS objectCBAddr;

        //std::vector<InstanceData> instanceData;
        //SharedPtr<FD3D12Buffer> instanceBuffer;

        D3D12_VERTEX_BUFFER_VIEW instanceVBV;
        size_t instanceCount = 0;

        uint32_t localHeapOffset = 0;
    };



    struct BuildData {
        std::vector<DrawCmd> cmds;

        void Clear()
        {
            cmds.clear();
        }
    };
     
    struct PassContext {
        BuildData data;
        GPUResources res;
    }; 

    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void EndFrame(PassContext& passCtx) noexcept;
}
 

  
namespace Lit
{  
    using PassContext = Mesh::PassContext;  
    using ObjectCB = Mesh::ObjectCB;
     
    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept; 

}

 
namespace Passes {

    inline RenderPassDesc ShadowPassDesc = {
       .passTag = "Shadow",
       .colorFormats = { },
       .depthFormat = DXGI_FORMAT_D32_FLOAT,
       .enableDepth = true,
       .enableBlend = false,
       .cullMode = D3D12_CULL_MODE_NONE,
    };

}

namespace Materials { 

    inline MaterialDesc ShadowMaterialDesc = {
        .name = "Shadow",
        .shaderTag = "Lit", 

        .enableAlphaBlend = false,
        .doubleSided = false,
        .depthWrite = true
    };
} 



namespace Shadow
{ 
    using PassContext = Mesh::PassContext; 
    using ObjectCB = Mesh::ObjectCB;  

    struct StaticMeshShadowVertex
    {
        Float3 position;
    };

    template<>
    struct VertexLayoutTraits<StaticMeshShadowVertex> {
        static constexpr bool is_specialized = true;
        static constexpr auto attributes = std::to_array<VertexAttribute>({
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshShadowVertex, position) }
            });
    };  
     

    void Init(const RendererContext* ctx,PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept; 

}

namespace Passes {

    inline RenderPassDesc GBufferPassDesc = {
       .passTag = "GBuffer",
	   .colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT },
       .depthFormat = DXGI_FORMAT_D32_FLOAT,
       .enableDepth = true,
       .enableBlend = false,
       .cullMode = D3D12_CULL_MODE_NONE,
    };

}

namespace Materials {

    inline MaterialDesc PBRMaterialDesc = {
        .name = "PBR",
        .shaderTag = "PBR",

        .enableAlphaBlend = false,
        .doubleSided = false,
        .depthWrite = true
    };
}

namespace GBuffer
{ 
    using ObjectCB = Mesh::ObjectCB; 

	struct GPUResources : public Mesh::GPUResources{

        //material CB:
		SharedPtr<FRingBufferAllocator<Materials::PBRMaterialCB>> matCBAllocator;
	};

	struct PassContext {
		Mesh::BuildData data;
		GPUResources res;
	};

    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept;

}