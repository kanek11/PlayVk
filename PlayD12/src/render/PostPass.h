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

#include "UIPass.h"


struct RendererContext;

namespace Passes {

    inline RenderPassDesc PBRShadingPassDesc = {
        .passTag = "Shading",
        .colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
        .depthFormat = DXGI_FORMAT_UNKNOWN,
        .enableDepth = false,
        .cullMode = D3D12_CULL_MODE_NONE
    };

}

namespace Materials {

    inline MaterialDesc PBRShadingMaterialDesc = {
    .shaderTag = "PBR",

    .enableAlphaBlend = true,
    .doubleSided = false,
    .depthWrite = false
    };
}

 

namespace PBR {

    using BuildData = SS::BuildData;
    using Vertex = SS::Vertex; 
    using DrawCmd = SS::DrawCmd; 
    using GPUResources = SS::GPUResources;

    struct PassContext {
        BuildData data;
        GPUResources res;
    }; 
     
    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept;

}



namespace Compute {



	struct PassContext {
		BuildData data;
		GPUResources res;
	};
	void Init(const RendererContext* ctx, PassContext& passCtx);
	void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;
	void BeginFrame(PassContext& passCtx) noexcept;
	void EndFrame(PassContext& passCtx) noexcept;
}