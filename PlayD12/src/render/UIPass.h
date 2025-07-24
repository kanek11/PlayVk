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

    inline RenderPassDesc UIPassDesc = {
        .passTag = "UI",
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
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


struct UIVertex {
    Float2 position;
    Float2 UV;
    Float4 color;
};

template<>
struct VertexLayoutTraits<UIVertex> {

    static constexpr bool is_specialized = true; 
	static constexpr auto attributes = std::to_array<VertexAttribute>({
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(UIVertex, position) },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(UIVertex, UV) },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(UIVertex, color) }
		});
};


//per object context
struct UIDrawCmd {
    uint32_t indexOffset, indexCount;
    //uint32_t textureSlot;
    bool useAtlas; 

    uint32_t heapOffset = 0;  

};

//all the data for a single batch / pass execution;
//basically transient;
struct UIBatchData {
    std::vector<UIVertex> vertices;
    std::vector<INDEX_FORMAT> indices;
    std::vector<UIDrawCmd> cmds;

    void Clear()
    { 
        cmds.clear();
		indices.clear();
		vertices.clear();
    }
};

//bug fix: make sure 256 byte aligned
struct UISettingsCB
{
    int useTexture = 1;
    float padding[63];
};
static_assert(sizeof(UISettingsCB) % 256 == 0, "UISettingsCB must be 256-byte aligned");


//basically the CPU side;
struct UIRenderData {
    UIBatchData batchData;
    UISettingsCB settings;

    bool dirty = false; 

    SharedPtr<FontAtlas> font;
};


//persistant
struct UIGPUResources {
    SharedPtr<FD3D12Buffer> batchVB;
    SharedPtr<FD3D12Buffer> batchIB;
    SharedPtr<FD3D12ShaderPermutation> shader;
    ComPtr<ID3D12PipelineState> pso; 


    uint32_t heapOffset = 0; 

    //SharedPtr<FD3D12Buffer> constantBuffer;
    SharedPtr<FRingBufferAllocator<UISettingsCB>> cbAllocator;
};
 


struct UIPassContext {
    UIRenderData data;
    UIGPUResources res;
};

 
void Init(const RenderPassInitContext& ctx, UIPassContext& passCtx); 
void FlushAndRender(ID3D12GraphicsCommandList* cmdList , const UIPassContext& passCtx) noexcept;

void BeginFrame(); 
void EndFrame(UIPassContext& passCtx) noexcept;

}