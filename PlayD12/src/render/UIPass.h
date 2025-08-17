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
#include "UI.h"


struct RendererContext;
 
struct FQuadDesc {
    FRect  rect;
    Float2 uvTL = { 0,0 };
    Float2 uvBR = { 1,1 };
    Float4 color = Color::White;
    bool   useAtlas = false;

};
//Screen space;
namespace SS {

    struct Vertex {
        Float2 position;
        Float2 UV;
        Float4 color;
    };

    template<>
    struct VertexLayoutTraits<Vertex> {

        static constexpr bool is_specialized = true;
        static constexpr auto attributes = std::to_array<VertexAttribute>({
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(Vertex, position) },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(Vertex, UV) },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, color) }
            });
    };


    //per object context
    struct DrawCmd {
        uint32_t indexOffset, indexCount;
        //uint32_t textureSlot;

        uint32_t heapOffset = 0;
    };

    //basically the CPU side;
    struct BuildData { 
        std::vector<DrawCmd> cmds;
        std::vector<Vertex> vertices;
        std::vector<INDEX_FORMAT> indices;

        std::vector<FQuadDesc> pendings;

        virtual void ResetFrame()
        {
            cmds.clear();
            indices.clear();
            vertices.clear();

            pendings.clear();
        }
    };

     
  
    struct GPUResources {
        SharedPtr<FD3D12ShaderPermutation> shader;
        ComPtr<ID3D12PipelineState> pso;

        std::optional<uint32_t> baseHeapOffset = 0;

        SharedPtr<FD3D12Buffer> batchVB;
		FBufferView batchVBV; 
        SharedPtr<FD3D12Buffer> batchIB; 
		FBufferView batchIBV;
    };

    //void BeginFrame(BuildData& data) noexcept;

    //void Init(const RendererContext* ctx, GPUResources& res,
    //    const RenderPassDesc& passDesc, const MaterialDesc& matDesc); 
 
}



namespace Passes {

    inline RenderPassDesc UIPassDesc = {
        .passTag = "UI",
        .colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
        .depthFormat = DXGI_FORMAT_UNKNOWN,
        .enableDepth = false,
        .cullMode = D3D12_CULL_MODE_NONE
    };

}

namespace Materials {

    inline  MaterialDesc UIMaterialDesc = {
    .shaderTag = "UI",

    .enableAlphaBlend = true,
    .doubleSided = false,
    .depthWrite = false
    };
}


namespace UI {

    using Vertex = SS::Vertex;
    using DrawCmd = SS::DrawCmd;
    using BuildData = SS::BuildData;
    using GPUResources = SS::GPUResources;

    //all the data for a single batch / pass execution;
    //basically transient; 
    //bug fix: make sure 256 byte aligned
    struct alignas(256) UISettingsCB
    {
        int useTexture = 0; 
    };
    //static_assert(sizeof(UISettingsCB) % 256 == 0, "CB must be 256-byte aligned");


    struct UIPassContext {
        BuildData data;
        GPUResources res;

        SharedPtr<FontAtlas> font;
        SharedPtr<FRingBufferAllocator<UISettingsCB>> cbAllocator;
    };


    void Init(const RendererContext* ctx, UIPassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const UIPassContext& passCtx) noexcept;

    void BeginFrame(UIPassContext& passCtx) noexcept;
    void EndFrame(UIPassContext& passCtx) noexcept;

}