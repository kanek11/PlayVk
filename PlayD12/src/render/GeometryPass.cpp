#include "PCH.h"

#include "GeometryPass.h"
#include "Renderer.h" 


//------------------------------
void Mesh::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    auto& frameData = Render::frameContext;

    auto& res = passCtx.res;
    auto& shader = res.shader;

    cmdList->SetPipelineState(res.PSO.Get());
    cmdList->SetGraphicsRootSignature(shader->GetRootSignature().Get());


    shader->SetDescriptorHeap(cmdList);
    shader->SetSceneRootCBV(
        cmdList,
        frameData->sceneCB->GetRawResource()
    );


    for (const auto& cmd : passCtx.data.cmds)
    {

        shader->SetDescriptorTables(cmdList, cmd.localHeapOffset);

        //cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetPrimitiveTopology(cmd.topology);

        cmdList->IASetVertexBuffers(0, 1, &cmd.vbv);
        cmdList->IASetVertexBuffers(1, 1, &cmd.instanceVBV);

        cmdList->IASetIndexBuffer(&cmd.ibv);

        //cmdList->DrawInstanced(3, 1, 0, 0);
        //cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);  
        auto indexCount = static_cast<UINT>(cmd.indexCount);
        auto instanceCount = static_cast<UINT>(cmd.instanceCount);
        cmdList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
    }
}

void Mesh::BeginFrame(PassContext& passCtx) noexcept
{
    auto& renderCtx = Render::rendererContext;
    auto& frameCtx = Render::frameContext;
    auto& graphCtx = Render::graphContext;

    if (frameCtx == nullptr || graphCtx == nullptr) {
        std::cerr << "render context is not inited" << std::endl;
        return;
    }

    auto& shader = passCtx.res.shader;
    auto& batchData = passCtx.data;

    assert(passCtx.res.baseHeapOffset.has_value());
    auto baseHeapOffset = passCtx.res.baseHeapOffset;

    auto& cbAllocator = passCtx.res.objCBAllocator;
    auto& instAllocator = passCtx.res.instanceBufferAllocator; 

    for (auto& proxy : frameCtx->staticMeshes) {

        auto currIndex = batchData.cmds.size();
        auto localHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());

        auto instBV = instAllocator->Upload(proxy.instanceData, proxy.instanceCount);

        //
        ObjectCB cb =
        {
            .modelMatrix = proxy.modelMatrix,
            .normalMatrix = ToFloat4x4(MMath::Transpose(MMath::Inverse3x3(ToFloat3x3(proxy.modelMatrix)))),
        };
        auto cbView = cbAllocator->Upload(&cb);
        shader->SetCBV("ObjectCB", GetCBVDesc(cbView), localHeapOffset);

        //todo: where to put this thing 
        if (!proxy.mesh->uploaded) {
            proxy.mesh->CreateGPUResource(renderCtx->device);
        }

        assert(proxy.mesh != nullptr && proxy.mesh->uploaded);
        Mesh::DrawCmd cmd =
        {
            .vbv = proxy.mesh->GetVertexBuffer()->GetVertexBufferView(),

            .topology = proxy.mesh->GetTopology(),

            .ibv = proxy.mesh->GetIndexBuffer()->GetIndexBufferView(),

            .indexCount = proxy.mesh->GetIndexCount(),

            .instanceVBV = GetVertexBufferView(instBV),
            .instanceCount = proxy.instanceCount, 

            .localHeapOffset = localHeapOffset,

        };

        batchData.cmds.push_back(cmd);
    }
}



void Mesh::EndFrame(PassContext& passCtx) noexcept
{
    passCtx.data.Clear();

    //reset the allocator:
    passCtx.res.objCBAllocator->Reset();
    passCtx.res.instanceBufferAllocator->Reset();
}

 


//------------------------------
void OverlayMesh::Init(const RendererContext* ctx, PassContext& passCtx)
{

    {
        passCtx.res.objCBAllocator
            = CreateShared<FRingBufferAllocator<ObjectCB>>(ctx->device, MaxStaticMesh);

        passCtx.res.instanceBufferAllocator
            = CreateShared<FRingBufferAllocator<InstanceData>>(ctx->device, MaxStaticMesh * 100);
    }



    // Create the pipeline state, which includes compiling and loading shaders.
    {
        auto& shader = passCtx.res.shader;

        ShaderPermutationKey key = {
            .shaderTag = "Debug",
            .passTag = "Volume", };

        shader = ctx->shaderManager->GetOrLoad(key);

        //shader->SetStaticSampler("baseMapSampler", Samplers::LinearWrap(0));
        //shader->SetStaticSampler("depthSampler", Samplers::LinearClamp(1));
        //shader->AutoSetSamplers();
        //shader->CreateRootSignature();

        passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxStaticMesh);

    }


    {
        auto& pso = passCtx.res.PSO;

        //assemble input layout:
        auto inputDescs = InputLayoutBuilder::Build<StaticMeshVertex>(0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
        auto perInstanceDescs = InputLayoutBuilder::Build<InstanceData>(1, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);

        inputDescs.insert(inputDescs.end(), perInstanceDescs.begin(), perInstanceDescs.end());

        //------------ 
        pso = ctx->psoManager->GetOrCreate(
            Materials::DebugMeshMaterialDesc,
            Passes::DebugMeshPassDesc,
            inputDescs
        );

    }

}

void OverlayMesh::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    Mesh::FlushAndRender(cmdList, passCtx); 
}

void OverlayMesh::BeginFrame(PassContext& passCtx) noexcept
{
    auto& renderCtx = Render::rendererContext;
    auto& frameCtx = Render::frameContext;
    auto& graphCtx = Render::graphContext;

    if (frameCtx == nullptr || graphCtx == nullptr) {
        std::cerr << "render context is not inited" << std::endl;
        return;
    }

    auto& shader = passCtx.res.shader;
    auto& batchData = passCtx.data;

    assert(passCtx.res.baseHeapOffset.has_value());
    auto baseHeapOffset = passCtx.res.baseHeapOffset;

    auto& cbAllocator = passCtx.res.objCBAllocator;
    auto& instAllocator = passCtx.res.instanceBufferAllocator;

    for (auto& proxy : frameCtx->transparentMeshes) {

        auto currIndex = batchData.cmds.size();
        auto localHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());

        auto instBV = instAllocator->Upload(proxy.instanceData, proxy.instanceCount);

        //
        ObjectCB cb =
        {
            .modelMatrix = proxy.modelMatrix,
            .normalMatrix = ToFloat4x4(MMath::Transpose(MMath::Inverse3x3(ToFloat3x3(proxy.modelMatrix)))),
        };
        auto cbView = cbAllocator->Upload(&cb);
        shader->SetCBV("ObjectCB", GetCBVDesc(cbView), localHeapOffset);

        //todo: where to put this thing 
        if (!proxy.mesh->uploaded) {
            proxy.mesh->CreateGPUResource(renderCtx->device);
        }

        assert(proxy.mesh != nullptr && proxy.mesh->uploaded);
        Mesh::DrawCmd cmd =
        {
            .vbv = proxy.mesh->GetVertexBuffer()->GetVertexBufferView(),

            .topology = proxy.mesh->GetTopology(),

            .ibv = proxy.mesh->GetIndexBuffer()->GetIndexBufferView(),

            .indexCount = proxy.mesh->GetIndexCount(),

            .instanceVBV = GetVertexBufferView(instBV),
            .instanceCount = proxy.instanceCount,

            .localHeapOffset = localHeapOffset,

        };

        batchData.cmds.push_back(cmd);
    }

}

void OverlayMesh::EndFrame(PassContext& passCtx) noexcept
{
    Mesh::EndFrame(passCtx);
} 

//------------------------------
void Shadow::Init(const RendererContext* ctx, PassContext& passCtx)
{

    {
        passCtx.res.objCBAllocator
            = CreateShared<FRingBufferAllocator<ObjectCB>>(ctx->device, MaxStaticMesh);

        passCtx.res.instanceBufferAllocator
            = CreateShared<FRingBufferAllocator<InstanceData>>(ctx->device, MaxStaticMesh * 100);
    }


    //shader:
    {
        auto& shader = passCtx.res.shader;

        ShaderPermutationKey key = {
       .shaderTag = "Lit",
       .passTag = "Shadow", };

        shader = ctx->shaderManager->GetOrLoad(key);
        //shader->CreateRootSignature();

        passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxStaticMesh);
    }

    //PSO
    {
        auto& pso = passCtx.res.PSO;

        // Assemble input layout for shadow pass.
        auto inputDescs = InputLayoutBuilder::Build<StaticMeshShadowVertex>();

        //------------ 
        pso = ctx->psoManager->GetOrCreate(
            Materials::ShadowMaterialDesc,
            Passes::ShadowPassDesc,
            inputDescs
        );

    }

}

void Shadow::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    Mesh::FlushAndRender(cmdList, passCtx);
}

void Shadow::BeginFrame(PassContext& passCtx) noexcept
{
    Mesh::BeginFrame(passCtx);
}

void Shadow::EndFrame(PassContext& passCtx) noexcept
{
    Mesh::EndFrame(passCtx);
}



void GBuffer::Init(const RendererContext* ctx, PassContext& passCtx)
{
    {
        passCtx.res.objCBAllocator
            = CreateShared<FRingBufferAllocator<ObjectCB>>(ctx->device, MaxStaticMesh);

        passCtx.res.instanceBufferAllocator
            = CreateShared<FRingBufferAllocator<InstanceData>>(ctx->device, MaxStaticMesh * 100);

        passCtx.res.matCBAllocator
            = CreateShared<FRingBufferAllocator<Materials::PBRMaterialCB>>(ctx->device, MaxStaticMesh);

    }

    {
        auto& shader = passCtx.res.shader;

        ShaderPermutationKey key = {
            .shaderTag = "PBR",
            .passTag = "GBuffer", };

        shader = ctx->shaderManager->GetOrLoad(key);

        //shader->SetStaticSampler("linearWrapSampler", Samplers::LinearWrap(0)); 
        //shader->AutoSetSamplers();
        //shader->CreateRootSignature();

        passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap(MaxStaticMesh);

    }


    {
        auto& pso = passCtx.res.PSO;

        //assemble input layout:
        auto inputDescs = InputLayoutBuilder::Build<StaticMeshVertex>(0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
        auto perInstanceDescs = InputLayoutBuilder::Build<InstanceData>(1, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);

        inputDescs.insert(inputDescs.end(), perInstanceDescs.begin(), perInstanceDescs.end());

        //------------ 
        pso = ctx->psoManager->GetOrCreate(
            Materials::PBRMaterialDesc,
            Passes::GBufferPassDesc,
            inputDescs
        );

    }
}

void GBuffer::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    auto& frameData = Render::frameContext;

    auto& res = passCtx.res;
    auto& shader = res.shader;

    cmdList->SetPipelineState(res.PSO.Get());
    cmdList->SetGraphicsRootSignature(shader->GetRootSignature().Get());


    shader->SetDescriptorHeap(cmdList);
    shader->SetSceneRootCBV(
        cmdList,
        frameData->sceneCB->GetRawResource()
    );


    for (const auto& cmd : passCtx.data.cmds)
    {

        shader->SetDescriptorTables(cmdList, cmd.localHeapOffset);

        //cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetPrimitiveTopology(cmd.topology);

        cmdList->IASetVertexBuffers(0, 1, &cmd.vbv);
        cmdList->IASetVertexBuffers(1, 1, &cmd.instanceVBV);

        cmdList->IASetIndexBuffer(&cmd.ibv);

        //cmdList->DrawInstanced(3, 1, 0, 0);
        //cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);  
        auto indexCount = static_cast<UINT>(cmd.indexCount);
        auto instanceCount = static_cast<UINT>(cmd.instanceCount);
        cmdList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
    }
}

void GBuffer::BeginFrame(PassContext& passCtx) noexcept
{
    auto& renderCtx = Render::rendererContext;
    auto& frameCtx = Render::frameContext;
    auto& graphCtx = Render::graphContext;

    if (frameCtx == nullptr || graphCtx == nullptr) {
        std::cerr << "render context is not inited" << std::endl;
        return;
    }

    auto& shader = passCtx.res.shader;
    auto& batchData = passCtx.data;

    assert(passCtx.res.baseHeapOffset.has_value());
    auto baseHeapOffset = passCtx.res.baseHeapOffset;

    auto& cbAllocator = passCtx.res.objCBAllocator;
    auto& instAllocator = passCtx.res.instanceBufferAllocator;


    for (auto& proxy : frameCtx->staticMeshes) {

        auto currIndex = batchData.cmds.size();
        auto localHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());

        auto instBV = instAllocator->Upload(proxy.instanceData, proxy.instanceCount);

        ObjectCB cb =
        {
            .modelMatrix = proxy.modelMatrix,
            .normalMatrix = ToFloat4x4(MMath::Transpose(MMath::Inverse3x3(ToFloat3x3(proxy.modelMatrix)))),
        };
        auto objCBV = cbAllocator->Upload(&cb);
        shader->SetCBV("ObjectCB", GetCBVDesc(objCBV), localHeapOffset);


        Materials::PBRMaterialCB matParams = proxy.material->materialCB;
        auto matCBView = passCtx.res.matCBAllocator->Upload(&matParams);
        shader->SetCBV("MaterialCB", GetCBVDesc(matCBView), localHeapOffset);
        for (auto& [name, texName] : proxy.material->textures)
        {
            auto& tex = graphCtx->loadedTextures[texName];
            if (tex)
            {
                shader->SetSRV(name, tex->GetRawResource(), tex->GetSRVDesc(), localHeapOffset);
            }
            else
            {
                std::cerr << "didn't find load tex:" << texName << std::endl;
            }

        }

        //todo: where to put this thing 
        if (!proxy.mesh->uploaded) {
            proxy.mesh->CreateGPUResource(renderCtx->device);
        }

        assert(proxy.mesh != nullptr && proxy.mesh->uploaded); 
        //todo 
        Mesh::DrawCmd cmd =
        {
            .vbv = proxy.mesh->GetVertexBuffer()->GetVertexBufferView(),

            .topology = proxy.mesh->GetTopology(),

            .ibv = proxy.mesh->GetIndexBuffer()->GetIndexBufferView(),

            .indexCount = proxy.mesh->GetIndexCount(),

            .instanceVBV = GetVertexBufferView(instBV),
            .instanceCount = proxy.instanceCount,


            .localHeapOffset = localHeapOffset,

        };

        batchData.cmds.push_back(cmd);
    }

}

void GBuffer::EndFrame(PassContext& passCtx) noexcept
{
    passCtx.data.Clear();
    //reset the allocator:
    passCtx.res.objCBAllocator->Reset();
    passCtx.res.matCBAllocator->Reset();
    passCtx.res.instanceBufferAllocator->Reset();
}