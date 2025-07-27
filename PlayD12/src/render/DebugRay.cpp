#include "PCH.h"
#include "DebugRay.h"

#include "Renderer.h"

void DebugDraw::AddRay(const Float3& origin, const Float3& direction,
    const Float4& color0,
    const Float4& color1
    )
{

    //this->AddLine(origin, origin + direction, color0, color1);

}

void DebugDraw::AddLine(const Float3& start, const Float3& end,
    const Float4& color0,
    const Float4& color1)
{
//#if defined(_DEBUG)
//    DebugLineVertex vert0 = {
//        .position = start,
//        .color = color0,
//    };
//    DebugLineVertex vert1 = {
//       .position = end,
//       .color = color1,
//    };
//
//    m_lineData.push_back(vert0);
//    m_lineData.push_back(vert1);
//#endif
}

void DebugDraw::FlushAndRender(ID3D12GraphicsCommandList* cmdList)
{
    if (m_lineData.empty()) return;

    //std::cout << "flush debug ray num: " << m_lineData.size() /2 << '\n';

    //auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(1280), static_cast<float>(720));
    //auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(1280), static_cast<LONG>(720));

    //
    cmdList->SetPipelineState(m_PSO.Get());
    cmdList->SetGraphicsRootSignature(m_shader->GetRootSignature().Get());

    //cmdList->RSSetViewports(1, &viewport);
    //cmdList->RSSetScissorRects(1, &scissorRect);


    // Set the descriptor heap for the command list
    m_shader->SetDescriptorHeap(cmdList);
    m_shader->SetDescriptorTables(cmdList, heapOffset);
    //or, e.g., cmd->SetGraphicsRootConstantBufferView(...)  

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());

    cmdList->DrawInstanced(static_cast<UINT>(m_lineData.size()), 1, 0, 0);

    m_lineData.clear();
}

DebugDraw& DebugDraw::Get()
{
    static DebugDraw instance;
    return instance;
}

void DebugDraw::Init(const RendererContext* ctx)
{
    size_t MaxVertices = 2 * MaxLines;

    // Create vertex buffer for debug lines
    m_vertexBuffer = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
        sizeof(DebugLineVertex) * MaxVertices,
        DXGI_FORMAT_UNKNOWN,
        sizeof(DebugLineVertex),
        EBufferUsage::Upload | EBufferUsage::Vertex
        });

    // Reserve CPU space for lines
    m_lineData.reserve(MaxVertices);


    //shader perm:
    ShaderPermutationKey key = {
        .shaderTag = "Debug",
        .passTag = "Line",
    };
    m_shader = ctx->shaderManager->GetOrLoad(key);
    m_shader->CreateRootSignature();

    //input 
    auto inputDesc = InputLayoutBuilder::Build<DebugLineVertex>();

    // Create PSO for debug rendering
    m_PSO = ctx->psoManager->GetOrCreate(
        m_materialDesc,
        m_renderPassDesc,
        inputDesc
    );


    //the cosntant buffer:
    m_CB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
        sizeof(MVPConstantBuffer),
        DXGI_FORMAT_UNKNOWN, // Not used for constant buffers
        256, // Alignment
        EBufferUsage::Upload | EBufferUsage::Constant
        });

    MVPConstantBuffer cbData = {};
    cbData.modelMatrix = MMath::MatrixIdentity<float, 4>();
    m_CB->UploadData(&cbData, sizeof(MVPConstantBuffer));

    //set CBV: 
    heapOffset = m_shader->RequestAllocationOnHeap();
    m_shader->SetCBV("MVPConstantBuffer",
        m_CB->GetCBVDesc(),
        heapOffset);

}

void DebugDraw::OnUpdate(float delta, const Float4x4& pv)
{
    // Upload the constant buffer data
    MVPConstantBuffer cbData = {};
    cbData.projectionViewMatrix = pv;
    m_CB->UploadData(&cbData, sizeof(MVPConstantBuffer));

    m_vertexBuffer->UploadData(m_lineData.data(), m_lineData.size() * sizeof(DebugLineVertex));
}

