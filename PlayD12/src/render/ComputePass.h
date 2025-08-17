#pragma once

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

namespace Compute {

    struct PassInitParams {
		std::string shaderTag;
		std::string passTag; 
    };


    struct DispatchCmd {
        uint32_t groupX = 8;
        uint32_t groupY = 8;
        uint32_t groupZ = 1;

        uint32_t heapOffset = 0;
    };

    struct GPUResources {
        SharedPtr<FD3D12ShaderPermutation> shader;
        ComPtr<ID3D12PipelineState> pso;

        std::optional<uint32_t> baseHeapOffset = 0;
    };

    struct ComputeContext {
        DispatchCmd cmd;
        GPUResources res;
    };
     
    void Init(const RendererContext* ctx,
        ComputeContext& passCtx,
        const std::string& shaderTag,
        const std::string& passTag
    );
    void DispatchCompute(ID3D12GraphicsCommandList* cmdList, 
        const ComputeContext& ctx) noexcept;

    //void BeginFrame(ComputeContext& passCtx) noexcept;
    //void EndFrame(PassContext& passCtx) noexcept;
}