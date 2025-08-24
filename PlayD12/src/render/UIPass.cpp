#include "PCH.h" 

#include "UIPass.h"
#include "Renderer.h" 

#include "Application.h"

#include "Loader.h"

using namespace Buffer;

namespace UI {

    void Init(const RendererContext* ctx, UIPassContext& passCtx)
    {
        auto& graphCtx = Render::graphContext;
        //----------------------
        auto& font = passCtx.font;
        font = CreateShared<FontAtlas>();
        if (auto& tex = graphCtx->loadedTextures["ASCII_16x6.png"]; tex) {
            font->texture = tex;
            auto& desc = tex->GetDesc();
            float cellWidth = (desc.width - 1) / 16.0f;
            float cellHeight = (desc.height - 1) / 6.0f;
            font->LoadGridAtlas(cellWidth, cellHeight, 16, 6);
        }
        else
        {
            std::cerr << "fail to load atlas" << std::endl;
        }

        //----------------------
        size_t MaxVertices = 4 * MaxUIBatch;
        size_t MaxIndices = 6 * MaxUIBatch;

        // Create vertex buffer for debug lines
        passCtx.res.batchVB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
            sizeof(Vertex) * MaxVertices,
            EBufferUsage::Upload | EBufferUsage::Vertex
            });

        passCtx.res.batchVBV = Buffer::CreateBVStructured<Vertex>(passCtx.res.batchVB);

        passCtx.res.batchIB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
        sizeof(INDEX_FORMAT) * MaxIndices,
        EBufferUsage::Upload | EBufferUsage::Index
            });

        passCtx.res.batchIBV = Buffer::CreateBVStructured<INDEX_FORMAT>(
            passCtx.res.batchIB, INDEX_FORMAT_DX
        );



        //shader perm:
        ShaderPermutationKey key = {
           Materials::UIMaterialDesc.shaderTag,
           Passes::UIPassDesc.passTag,
        };

        auto& shader = passCtx.res.shader;
        shader = ctx->shaderManager->GetOrLoad(key);
        //shader->SetStaticSampler("linearWrapSampler", Samplers::LinearWrap());
        //shader->CreateRootSignature(); 

        //input desc:
        auto inputDesc = InputLayoutBuilder::Build<Vertex>(0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);


        // Create PSO for debug rendering
        passCtx.res.pso = ctx->psoManager->GetOrCreate(
            Materials::UIMaterialDesc,
            Passes::UIPassDesc,
            inputDesc
        );


        passCtx.cbAllocator = CreateShared<FRingBufferAllocator<UISettingsCB>>(ctx->device, MaxUIBatch);


        passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxUIBatch);

    }

    void BeginFrame(UIPassContext& passCtx) noexcept
    {
        auto& graphCtx = Render::graphContext;

        int screenWidth = GameApplication::GetInstance()->GetWidth();
        int screenHeight = GameApplication::GetInstance()->GetHeight();

        auto& shader = passCtx.res.shader;

        auto& cbAllocator = passCtx.cbAllocator;

        assert(passCtx.res.baseHeapOffset.has_value());
        auto baseHeapOffset = passCtx.res.baseHeapOffset;

        //auto& baseColorTex = graphCtx->loadedTextures["A"]
        auto& batchData = passCtx.data;

        uint32_t currIndex{ 0 };
        for (auto& proxy : passCtx.data.pendings) {
            Float2 tl = ScreenToNDC(proxy.rect.x, proxy.rect.y, screenWidth, screenHeight);
            Float2 tr = ScreenToNDC(proxy.rect.x + proxy.rect.w, proxy.rect.y, screenWidth, screenHeight);
            Float2 bl = ScreenToNDC(proxy.rect.x, proxy.rect.y + proxy.rect.h, screenWidth, screenHeight);
            Float2 br = ScreenToNDC(proxy.rect.x + proxy.rect.w, proxy.rect.y + proxy.rect.h, screenWidth, screenHeight);

            uint32_t baseVertex = static_cast<uint32_t>(batchData.vertices.size());
            batchData.vertices.push_back({ tl, {proxy.uvTL.x(), proxy.uvTL.y()}, {proxy.color.x(), proxy.color.y(), proxy.color.z(), proxy.alpha.x() } }); // 0
            batchData.vertices.push_back({ tr, {proxy.uvBR.x(), proxy.uvTL.y()}, {proxy.color.x(), proxy.color.y(), proxy.color.z(), proxy.alpha.y() } }); // 1
            batchData.vertices.push_back({ bl, {proxy.uvTL.x(), proxy.uvBR.y()}, {proxy.color.x(), proxy.color.y(), proxy.color.z(), proxy.alpha.z() } }); // 2
            batchData.vertices.push_back({ br, {proxy.uvBR.x(), proxy.uvBR.y()}, {proxy.color.x(), proxy.color.y(), proxy.color.z(), proxy.alpha.w() } }); // 3

            uint32_t baseIndex = static_cast<uint32_t>(batchData.indices.size());
            batchData.indices.push_back(baseVertex + 0);
            batchData.indices.push_back(baseVertex + 1);
            batchData.indices.push_back(baseVertex + 2);
            batchData.indices.push_back(baseVertex + 2);
            batchData.indices.push_back(baseVertex + 1);
            batchData.indices.push_back(baseVertex + 3);

            UI::UISettingsCB uiSettings = {
                .useTexture = static_cast<int>(proxy.useTexture),
                .opacity = proxy.opacity
            };

            auto objectHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());


            auto bufferView = cbAllocator->Upload(&uiSettings);
            shader->SetCBV("UISettingsCB", Buffer::MakeCBVDesc(bufferView), objectHeapOffset);

            if (proxy.useTexture) {
                auto& baseColorTex = graphCtx->loadedTextures[proxy.texName];
                shader->SetSRV("baseColorMap", baseColorTex->GetRawResource(), baseColorTex->GetSRVDesc(), objectHeapOffset);
            }


            UI::DrawCmd cmd = {
                .indexOffset = baseIndex,
                .indexCount = 6,
                .heapOffset = static_cast<uint32_t>(objectHeapOffset) ,
            };

            batchData.cmds.push_back(cmd);
            currIndex++;
        }


    }

    void EndFrame(UIPassContext& passCtx) noexcept
    {
        passCtx.data.ResetFrame();

        //reset the allocator:
        passCtx.cbAllocator->Reset();

    }

    void FlushAndRender(ID3D12GraphicsCommandList* cmdList, const UIPassContext& passCtx) noexcept
    {
        auto& batchData = passCtx.data;
        if (batchData.cmds.empty()) return;

        passCtx.res.batchVB->UploadData(passCtx.data.vertices.data(), passCtx.data.vertices.size() * sizeof(Vertex));
        passCtx.res.batchIB->UploadData(passCtx.data.indices.data(), passCtx.data.indices.size() * sizeof(INDEX_FORMAT));


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

        cmdList->IASetVertexBuffers(0, 1, &MakeVBV(passCtx.res.batchVBV));
        cmdList->IASetIndexBuffer(&MakeIBV(passCtx.res.batchIBV));

        for (auto& cmd : batchData.cmds) {

            passCtx.res.shader->SetDescriptorTables(cmdList, cmd.heapOffset);

            auto indexCount = cmd.indexCount;
            auto indexOffset = cmd.indexOffset;
            cmdList->DrawIndexedInstanced((UINT)indexCount, 1, (UINT)indexOffset, 0, 0);
        }

    }

}

//void SS::Init(const RendererContext* ctx, GPUResources& res)
//{
//}