#include "PCH.h"

#include "PostPass.h"

#include "Renderer.h" 

#include "Application.h"

using namespace Buffer;


void PBR::Init(const RendererContext* ctx, PassContext& passCtx)
{
    //----------------------
    size_t MaxVertices = 4;
    size_t MaxIndices = 6;

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
        Materials::PBRShadingMaterialDesc.shaderTag,
        Passes::PBRShadingPassDesc.passTag,
    };

    auto& shader = passCtx.res.shader;
    shader = ctx->shaderManager->GetOrLoad(key);
    //shader->SetStaticSampler("linearWrapSampler", Samplers::LinearWrap(0));
    //shader->SetStaticSampler("depthSampler", Samplers::LinearClamp(1));
    //shader->AutoSetSamplers();
    //shader->CreateRootSignature();


    //input desc:
    auto inputDesc = InputLayoutBuilder::Build<Vertex>(0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);

    // Create PSO for debug rendering
    passCtx.res.pso = ctx->psoManager->GetOrCreate(
        Materials::PBRShadingMaterialDesc,
        Passes::PBRShadingPassDesc,
        inputDesc
    );

    passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxUIBatch);
}
 

void PBR::BeginFrame(PassContext& passCtx) noexcept
{

    int screenWidth = GameApplication::GetInstance()->GetWidth();
    int screenHeight = GameApplication::GetInstance()->GetHeight();

    FRect screenRect = {
    .x = 0, .y = 0,
    .w = screenWidth, .h = screenHeight
    };

    auto screenQuad = FQuadDesc
    {
        .rect = screenRect,
        .uvTL = {0,0},
        .uvBR = {1,1},
    }; 

    passCtx.data.pendings = { screenQuad };


    auto& shader = passCtx.res.shader;

    assert(passCtx.res.baseHeapOffset.has_value());
    auto baseHeapOffset = passCtx.res.baseHeapOffset;

    auto& graphCtx = Render::graphContext;
    auto& gbuffers = graphCtx->gbuffers;

    auto& batchData = passCtx.data;

    uint32_t currIndex{ 0 };
    for (auto& proxy : passCtx.data.pendings) {
        Float2 tl = ScreenToNDC(proxy.rect.x, proxy.rect.y, screenWidth, screenHeight);
        Float2 tr = ScreenToNDC(proxy.rect.x + proxy.rect.w, proxy.rect.y, screenWidth, screenHeight);
        Float2 bl = ScreenToNDC(proxy.rect.x, proxy.rect.y + proxy.rect.h, screenWidth, screenHeight);
        Float2 br = ScreenToNDC(proxy.rect.x + proxy.rect.w, proxy.rect.y + proxy.rect.h, screenWidth, screenHeight);

        uint32_t baseVertex = static_cast<uint32_t>(batchData.vertices.size());
        batchData.vertices.push_back({ tl, {proxy.uvTL.x(), proxy.uvTL.y()}, proxy.color }); // 0
        batchData.vertices.push_back({ tr, {proxy.uvBR.x(), proxy.uvTL.y()}, proxy.color }); // 1
        batchData.vertices.push_back({ bl, {proxy.uvTL.x(), proxy.uvBR.y()}, proxy.color }); // 2
        batchData.vertices.push_back({ br, {proxy.uvBR.x(), proxy.uvBR.y()}, proxy.color }); // 3

        uint32_t baseIndex = static_cast<uint32_t>(batchData.indices.size());
        batchData.indices.push_back(baseVertex + 0);
        batchData.indices.push_back(baseVertex + 1);
        batchData.indices.push_back(baseVertex + 2);
        batchData.indices.push_back(baseVertex + 2);
        batchData.indices.push_back(baseVertex + 1);
        batchData.indices.push_back(baseVertex + 3);

        UI::UISettingsCB uiSettings = {
            .useTexture = static_cast<int>(proxy.useAtlas),
        };
         
        auto objectHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());

        for (const auto& [name, gbufferTex] : gbuffers)
        {
            shader->SetSRV(name, gbufferTex->GetRawResource(), gbufferTex->GetSRVDesc(), objectHeapOffset);
        }

        if (graphCtx->sceneDepth)
        {
            shader->SetSRV("ds_viewDepth", graphCtx->sceneDepth->GetRawResource(), graphCtx->sceneDepth->GetSRVDesc(), objectHeapOffset);
        }

        if (graphCtx->shadowMap)
            shader->SetSRV("shadowMap", graphCtx->shadowMap->GetRawResource(), graphCtx->shadowMap->GetSRVDesc(), objectHeapOffset);

        if (auto& probe = graphCtx->probe; probe!=nullptr) {
            shader->SetSRV("skybox", probe->envMap->GetRawResource(), probe->envMap->GetSRVDesc(), objectHeapOffset);
            shader->SetSRV("irradianceMap", probe->diffuseIrradiance->GetRawResource(), probe->diffuseIrradiance->GetSRVDesc(), objectHeapOffset);
            shader->SetSRV("prefilterMap", probe->specularPrefilter->GetRawResource(), probe->specularPrefilter->GetSRVDesc(), objectHeapOffset);
 
            //auto srvDesc = probe->specularPrefilter->GetDesc();
            //std::cout << "srv miplevels:" << srvDesc.mipLevels << '\n';
            shader->SetSRV("brdfLUT", probe->brdfLUT->GetRawResource(), probe->brdfLUT->GetSRVDesc(), objectHeapOffset);

        }

        DrawCmd cmd = {
            .indexOffset = baseIndex,
            .indexCount = 6,
            .heapOffset = static_cast<uint32_t>(objectHeapOffset) ,
        };

        batchData.cmds.push_back(cmd);
		currIndex++;
    }
}

void PBR::EndFrame(PassContext& passCtx) noexcept
{
    passCtx.data.ResetFrame();
}

void PBR::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    auto& batchData = passCtx.data;
    if (batchData.cmds.empty()) return;

    passCtx.res.batchVB->UploadData(passCtx.data.vertices.data(), passCtx.data.vertices.size() * sizeof(Vertex));
    passCtx.res.batchIB->UploadData(passCtx.data.indices.data(), passCtx.data.indices.size() * sizeof(INDEX_FORMAT));


    auto& frameData = Render::frameContext;

    auto& res = passCtx.res;
    auto& shader = res.shader;

    cmdList->SetPipelineState(res.pso.Get());
    cmdList->SetGraphicsRootSignature(shader->GetRootSignature().Get());


    shader->SetDescriptorHeap(cmdList);
    shader->SetSceneRootCBV(
        cmdList,
        frameData->sceneCB->GetRawResource()
    );


    // Set the descriptor heap for the command list
    shader->SetDescriptorHeap(cmdList);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //cmdList->IASetVertexBuffers(0, 1, &passCtx.res.batchVB->GetVertexBufferView());
 //   cmdList->IASetIndexBuffer(&res.batchIB->GetIndexBufferView());
	//cmdList->IASetVertexBuffers(0, 1, &res.batchVB->GetVertexBufferView());
    cmdList->IASetIndexBuffer(&MakeIBV(res.batchIBV));
    cmdList->IASetVertexBuffers(0, 1, &MakeVBV(res.batchVBV));

    for (auto& cmd : batchData.cmds) {

        shader->SetDescriptorTables(cmdList, cmd.heapOffset);

        auto indexCount = cmd.indexCount;
        auto indexOffset = cmd.indexOffset;
        cmdList->DrawIndexedInstanced((UINT)indexCount, 1, (UINT)indexOffset, 0, 0);
    }

}