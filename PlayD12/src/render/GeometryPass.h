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
 

struct RendererContext;

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
 

//strip out the minimum to render a static mesh:
struct FStaticMeshProxy {
    Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();

    UStaticMesh* mesh;

    FMaterialProxy* material;


    InstanceData* instanceData;
    size_t instanceCount;
};
     

namespace Mesh{

    //struct SceneCB
    //{
    //    Float4x4  pvMatrix;
    //    Float4x4  light_pvMatrix;
    //    //padding:
    //    float padding[48];
    //};

    struct alignas(256) SceneCB
    {
        Float4x4  pvMatrix;
        Float4x4  light_pvMatrix;
        Float3 lightDir = Normalize(Float3(0.577f, 0.277f, 0.377f));
		float padding1 = 0.0f;  
        Float3 lightColor = Float3(1.0f, 1.0f, 1.0f);
		float padding2 = 0.0f;    

        SceneCB() {  
            Float3 lightPos = lightDir * 100.0f;   
            Float3 target = Float3(0.0f, 0.0f, 0.0f);

            // 4. Up vector (choose world up, e.g. Y axis)
            Float3 up = Float3(0.0f, 1.0f, 0.0f);

            // 5. Create light view matrix (left-handed)
            Float4x4 lightView = LookAtLH(lightPos, target, up);

            // 6. Create orthographic projection matrix (tweak width/height and near/far planes to cover scene)
            float orthoWidth = 50.0f;
            float orthoHeight = 50.0f;
            float nearZ = 0.1f;
            float farZ = 500.0f;

            Float4x4 lightProj = MMath::OrthographicLH(orthoWidth, orthoHeight, nearZ, farZ);
            //lightProj = Transpose(lightProj);

            // 7. Combine into light-space matrix
            this->light_pvMatrix = MMath::MatrixMultiply(lightProj, lightView);
        }
    };

    struct ObjectCB
    {
        Float4x4 modelMatrix;
        float padding[48];
    };

    static_assert((sizeof(SceneCB) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
    static_assert((sizeof(ObjectCB) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

    struct GPUResources {
        ComPtr<ID3D12PipelineState> PSO;
        SharedPtr<FD3D12ShaderPermutation> shader;

        SharedPtr<FD3D12Buffer> sceneCB;

        SharedPtr<FRingBufferAllocator<ObjectCB>> cbAllocator;

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
}
 

  
namespace Lit
{  
    using PassContext = Mesh::PassContext; 

    using SceneCB = Mesh::SceneCB;
    using ObjectCB = Mesh::ObjectCB;
     
    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept; 

}

 
namespace Passes {

    inline RenderPassDesc ShadowPassDesc = {
       .passTag = "Shadow",
       .colorFormat = DXGI_FORMAT_UNKNOWN, // no color
       .depthFormat = DXGI_FORMAT_D32_FLOAT,
       .enableDepth = true,
       .enableBlend = false,
       .cullMode = D3D12_CULL_MODE_BACK,
    };

}

namespace Materials { 

    inline MaterialDesc ShadowMaterialDesc = {
        .name = "Shadow",
        .shaderTag = "Lit",

        .textures = {
            {"baseMap", "default_shadow_texture"}, // Placeholder texture handle
        },

        .enableAlphaBlend = false,
        .doubleSided = false,
        .depthWrite = true
    };
} 



namespace Shadow
{ 
    using PassContext = Mesh::PassContext;
    using SceneCB = Mesh::SceneCB;
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


