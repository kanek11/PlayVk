
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


struct RendererContext; 

struct MVPConstantBuffer
{
    //XMFLOAT4X4 modelMatrix; // 64 bytes  
    //XMFLOAT4X4 viewProjectionMatrix; // 64 bytes 
    Float4x4 modelMatrix; // 64 bytes 
    Float4x4 projectionViewMatrix; // 64 bytes

    //padding:
    float padding[32];
};


static_assert((sizeof(MVPConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


//struct DebugLine {
//    Float3 start;
//    Float4 color0;
//    Float3 end;
//    Float4 color1;
//};

struct DebugLineVertex {
    Float3 position;
    Float4 color;
};

//
template<>
struct VertexLayoutTraits<DebugLineVertex> {
    static constexpr bool is_specialized = true;
    static constexpr auto attributes = std::to_array<VertexAttribute>({
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DebugLineVertex, position) },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(DebugLineVertex, color) }
        });
};


class DebugDraw {
public:
    static DebugDraw& Get();

    void Init(const RendererContext* ctx);

    void OnUpdate(float delta, const Float4x4& pv);

    void AddRay(const Float3& origin, const Float3& direction,
        const Float4& color0 = Float4(1.0f, 1.0f, 1.0f, 1.0f),
        const Float4& color1 = Float4(1.0f, 1.0f, 1.0f, 1.0f)
    );
    void AddLine(const Float3& start, const Float3& end,
        const Float4& color = Float4(1.0f, 0.0f, 0.0f, 1.0f),
        const Float4& color1 = Float4(1.0f, 1.0f, 1.0f, 1.0f));

    void FlushAndRender(ID3D12GraphicsCommandList* cmdList);

    //static_assert(sizeof(DebugLineVertex) == sizeof(float) * 7, "unexpected vertex size?");
private:
    void Clear()
    {
        m_lineData.clear();
    }
private:
    std::vector<DebugLineVertex> m_lineData;
    SharedPtr<FD3D12Buffer> m_vertexBuffer;
    SharedPtr<FD3D12ShaderPermutation> m_shader;
    ComPtr<ID3D12PipelineState> m_PSO;

    //MVP buffer:
    SharedPtr<FD3D12Buffer> m_CB;
    uint32_t heapOffset;


    MaterialDesc m_materialDesc = {
        .shaderTag = "Debug",

        .enableAlphaBlend = true,
        .doubleSided = true,
        .depthWrite = false
    };

    RenderPassDesc m_renderPassDesc = {
        .passTag = "Line",
        .colorFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
        .depthFormat = DXGI_FORMAT_UNKNOWN,
        .enableDepth = false,
        .cullMode = D3D12_CULL_MODE_NONE,
		.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
    };

};

