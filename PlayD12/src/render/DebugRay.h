
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

#include "StaticMeshProxy.h"


struct RendererContext; 


namespace Materials {
	inline MaterialDesc DebugRayMaterialDesc = {
		.shaderTag = "Debug",
		.enableAlphaBlend = true,
		.doubleSided = true,
		.depthWrite = false
	};
}

namespace Passes {
	inline RenderPassDesc DebugRayPassDesc = {
		.passTag = "Line",
		.colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
		.depthFormat = DXGI_FORMAT_UNKNOWN,
		.enableDepth = false,
		.cullMode = D3D12_CULL_MODE_NONE,
		.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
	};
}

namespace DebugDraw {

    //struct DebugLine {
//    Float3 start;
//    Float4 color0;
//    Float3 end;
//    Float4 color1;
//};

    struct Vertex {
        Float3 position;
        Float4 color;
    };
     
    template<>
    struct VertexLayoutTraits<Vertex> {
        static constexpr bool is_specialized = true;
        static constexpr auto attributes = std::to_array<VertexAttribute>({
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(Vertex, position) },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(Vertex, color) }
            });
    };


	struct GPUResources {
		SharedPtr<FD3D12ShaderPermutation> shader;
		ComPtr<ID3D12PipelineState> pso;
		std::optional<uint32_t> baseHeapOffset = 0;

		SharedPtr<FD3D12Buffer> batchVB; 
		FBufferView batchVBV;
	};

	struct BuildData { 
        std::vector<Vertex> vertices; 

		virtual void ResetFrame()
		{ 
			vertices.clear();
		}
	};

	struct PassContext {
		BuildData data;
		GPUResources res; 
		 
		SharedPtr<UMaterial> debugCubeMat = CreateShared<UMaterial>();
		SharedPtr<UStaticMesh> debugCubMesh = CreateShared<CubeMesh>();
		std::vector<InstanceData> debugCubeInstances = {
			{ Float3(0, 0, 0) }
		};

		std::vector<FStaticMeshProxy> debugMeshes;

		void ResetFrame() noexcept
		{
			data.ResetFrame();
			debugMeshes.clear(); 
		}
	};


    void Init(const RendererContext* ctx, PassContext& passCtx);

    void BeginFrame(PassContext& passCtx) noexcept;
	void EndFrame(PassContext& passCtx) noexcept;

	void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept;
}

namespace DebugDraw {
 
    void AddLine(const Float3& start, const Float3& end,
        const Float4& color = Float4(1.0f, 0.0f, 0.0f, 1.0f),
		std::optional<Float4> color1 = std::nullopt
	);
  
    void AddRay(const Float3& origin, const Float3& direction,
        const Float4& color0 = Float4(1.0f, 1.0f, 1.0f, 1.0f),
		std::optional<Float4> color1 = std::nullopt
    );

	void ClearFrame(); 

	void AddCube(const Float3& center, float size, std::optional<Float4> color = std::nullopt);
}
