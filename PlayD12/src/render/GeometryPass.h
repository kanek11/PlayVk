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

#include "ShaderParameters.h"

#include "StaticMeshProxy.h"

struct RendererContext;

namespace Passes {

    inline RenderPassDesc DebugMeshPassDesc = {
    .passTag = "Volume",
    .colorFormats = {DXGI_FORMAT_R8G8B8A8_UNORM},
    .depthFormat = DXGI_FORMAT_D32_FLOAT,
    .enableDepth = false, 
    .cullMode = D3D12_CULL_MODE_NONE,
    };

}

namespace Materials {

    inline MaterialDesc DebugMeshMaterialDesc = {
    .shaderTag = "Debug",
    .enableAlphaBlend = true,
    .doubleSided = true,
    .depthWrite = false,
    .enableWireFrame = false, 
    };
}
 

namespace Mesh {


    struct GPUResources {
        ComPtr<ID3D12PipelineState> PSO;
        SharedPtr<FD3D12ShaderPermutation> shader; 

        std::optional<uint32_t> baseHeapOffset = 0;

        SharedPtr<FRingBufferAllocator<ObjectCB>> objCBAllocator; 
        SharedPtr<FRingBufferAllocator<InstanceData>> instanceBufferAllocator;

		virtual void ResetFrame()
		{ 
			objCBAllocator->Reset();
			instanceBufferAllocator->Reset();
		}
    };


    struct DrawCmd { 
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_PRIMITIVE_TOPOLOGY topology;
        D3D12_INDEX_BUFFER_VIEW ibv;
        size_t indexCount;

        D3D12_VERTEX_BUFFER_VIEW instanceVBV;
        size_t instanceCount = 0;

        uint32_t localHeapOffset = 0;
    };



    struct BuildData {
        std::vector<DrawCmd> cmds;

        void ResetFrame()
        {
            cmds.clear();
        }
    };

    struct PassContext {
        BuildData data;
        GPUResources res;
    };

    struct PassInitParms {
		RenderPassDesc passDesc;
		MaterialDesc materialDesc; 
    };

    //void Init(const RendererContext* ctx, GPUResources& res, const PassInitParms& params);

    //void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;
	void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const BuildData& data, const GPUResources& res) noexcept;

    //void BeginFrame(PassContext& passCtx) noexcept;
	void BeginFrame(BuildData& data, const GPUResources& res,
		const std::vector <FStaticMeshProxy>& proxies) noexcept;

    void EndFrame(PassContext& passCtx) noexcept; 
}


namespace DebugMesh
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

    //struct StaticMeshShadowVertex
    //{
    //    Float3 position;
    //};

    //template<>
    //struct VertexLayoutTraits<StaticMeshShadowVertex> {
    //    static constexpr bool is_specialized = true;
    //    static constexpr auto attributes = std::to_array<VertexAttribute>({
    //        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(StaticMeshShadowVertex, position) }
    //        });
    //};


    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept;

}

namespace Passes {

    inline RenderPassDesc GBufferPassDesc = {
       .passTag = "GBuffer",
       .colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT },
       .rtvNames = {
        "rt0_albedo_ao",
        "rt1_normal_rough",
        "rt2_position_metallic",
       },
       .depthFormat = DXGI_FORMAT_D32_FLOAT,
       .enableDepth = true, 
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
	using BuildData = Mesh::BuildData;
    using GPUResources = Mesh::GPUResources;
  //  struct GPUResources : public Mesh::GPUResources {

  //      //material CB:
  //      SharedPtr<FRingBufferAllocator<Materials::PBRMaterialCB>> matCBAllocator;
  // 
		//virtual void ResetFrame() override
		//{
		//	Mesh::GPUResources::ResetFrame();

		//	matCBAllocator->Reset(); 
		//}
  //  };

    struct PassContext {
        BuildData data;
        GPUResources res;

        SharedPtr<FRingBufferAllocator<Materials::PBRMaterialCB>> matCBAllocator;

		void ResetFrame()
		{
			data.ResetFrame();
			res.ResetFrame();
			matCBAllocator->Reset();
		}
    };

    void Init(const RendererContext* ctx, PassContext& passCtx);
    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;

    void BeginFrame(PassContext& passCtx) noexcept;
    void EndFrame(PassContext& passCtx) noexcept;

}