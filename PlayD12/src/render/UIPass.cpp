#include "PCH.h" 

#include "UIPass.h"
#include "Renderer.h" 

 
namespace UI {

void Init(const RenderPassInitContext& ctx, UIPassContext& passCtx)
{
    //----------------------
	auto& font = passCtx.data.font;
    font = CreateShared<FontAtlas>(); 
    font->LoadTexture("assets/ASCII_10x10.jpg"); 

    auto metaInfo = font->imageData.value().metaInfo;
    //auto atlasData  = font->imageData.value().data;
    FTextureDesc atlasDesc =
        FTextureDesc{
        .width = static_cast<UINT>(metaInfo.width),
        .height = static_cast<UINT>(metaInfo.height),
        .format = metaInfo.format,
        .usages = {ETextureUsage::ShaderResource},
    };


    auto atlasTex = CreateShared<FD3D12Texture>(ctx.device, atlasDesc);
    font->texture = atlasTex;

    float cellWidth = (metaInfo.width - 1) / 10.0f;
    float cellHeight = (metaInfo.height - 1) / 10.0f;
    font->LoadGridAtlas(cellWidth, cellHeight, 10, 10);

    //font->LoadGridAtlas(80, 80, metaInfo.width ,metaInfo.height);



    //----------------------
    size_t MaxVertices = 4 * MaxUIBatch;
    size_t MaxIndices = 6 * MaxUIBatch;

    // Create vertex buffer for debug lines
    passCtx.res.batchVB = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
        sizeof(UIVertex) * MaxVertices,
        DXGI_FORMAT_UNKNOWN,
        sizeof(UIVertex),
        EBufferUsage::Upload | EBufferUsage::Vertex
        });


    passCtx.res.batchIB = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
    sizeof(INDEX_FORMAT) * MaxIndices,
    DXGI_FORMAT_R16_UINT,
    sizeof(INDEX_FORMAT),
    EBufferUsage::Upload | EBufferUsage::Index
        });


    //shader perm:
    ShaderPermutationKey key = {
        .shaderTag = "UI",
        .passTag = "UI",
    };

	auto& shader = passCtx.res.shader;
    shader = ctx.m_shaderManager->GetOrLoad(key); 
    shader->SetStaticSampler("fontAtlasSampler", Samplers::LinearWrap());
    shader->CreateRootSignature();


    //input desc:
	auto inputDesc = InputLayoutBuilder::Build<UIVertex>(0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);


    // Create PSO for debug rendering
    passCtx.res.pso = ctx.m_psoManager->GetOrCreate(
        Materials::UIMaterialDesc,
        Passes::UIPassDesc,
        inputDesc
    );


	passCtx.res.cbAllocator = CreateShared<FRingBufferAllocator<UISettingsCB>>(ctx.device, MaxUIBatch);


    passCtx.res.heapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxUIBatch);

}

void BeginFrame()
{
}

void EndFrame(UIPassContext& passCtx) noexcept
{
    passCtx.data.dirty = false;

	auto& batchData = passCtx.data.batchData; 
    batchData.Clear();
     

    //reset the allocator:
    passCtx.res.cbAllocator->Reset();

}

void EndFrame()
{
}

void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const UIPassContext& passCtx) noexcept
{
	auto& batchData = passCtx.data.batchData;
    if (batchData.cmds.empty()) return;

    if (passCtx.data.dirty) {
        passCtx.res.batchVB->UploadData(passCtx.data.batchData.vertices.data(), passCtx.data.batchData.vertices.size() * sizeof(UIVertex));
        passCtx.res.batchIB->UploadData(passCtx.data.batchData.indices.data(), passCtx.data.batchData.indices.size() * sizeof(INDEX_FORMAT));
        //passCtx.data.dirty = false;
    }

    //std::cout << "flush UI draw cmd: " << m_data.cmds.size()  << '\n';

    //auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(1280), static_cast<float>(720));
    //auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(1280), static_cast<LONG>(720));

    //
    cmdList->SetPipelineState(passCtx.res.pso.Get());
    cmdList->SetGraphicsRootSignature(passCtx.res.shader->GetRootSignature().Get());

    //cmdList->RSSetViewports(1, &viewport);
    //cmdList->RSSetScissorRects(1, &scissorRect); 

    // Set the descriptor heap for the command list
    passCtx.res.shader->SetDescriptorHeap(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    cmdList->IASetVertexBuffers(0, 1, &passCtx.res.batchVB->GetVertexBufferView());
    cmdList->IASetIndexBuffer(&passCtx.res.batchIB->GetIndexBufferView());

    for (auto& cmd : batchData.cmds) {

		passCtx.res.shader->SetDescriptorTables(cmdList, cmd.heapOffset);

        auto indexCount = cmd.indexCount;
        auto indexOffset = cmd.indexOffset;
        cmdList->DrawIndexedInstanced((UINT)indexCount, 1, (UINT)indexOffset, 0, 0);
    }

    //batchData.Clear();\

}

}