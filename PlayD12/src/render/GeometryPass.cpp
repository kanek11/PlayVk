#include "PCH.h"

#include "GeometryPass.h"
#include "Renderer.h"

#include "StaticMeshActor.h"

 
void Mesh::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    auto& res = passCtx.res;
    auto& shader = res.shader;

    cmdList->SetPipelineState(res.PSO.Get());
    cmdList->SetGraphicsRootSignature(shader->GetRootSignature().Get());


    shader->SetDescriptorHeap(cmdList);
    shader->SetSceneRootCBV(
        cmdList,
        res.sceneCB->GetRawResource()
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




void Lit::Init(const RendererContext* ctx, PassContext& passCtx)
{

    {
        auto& sceneCB = passCtx.res.sceneCB;
        sceneCB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
            .SizeInBytes = sizeof(SceneCB),
            .Format = DXGI_FORMAT_UNKNOWN, // Not used for constant buffers 
            .StrideInBytes = sizeof(SceneCB),
            .Usage = EBufferUsage::Upload | EBufferUsage::Constant
            }); 

    } 


    {
        passCtx.res.cbAllocator
            = CreateShared<FRingBufferAllocator<ObjectCB>>(ctx->device, MaxStaticMesh);

        passCtx.res.instanceBufferAllocator
            = CreateShared<FRingBufferAllocator<InstanceData>>(ctx->device, MaxStaticMesh * 100);
    }



    // Create the pipeline state, which includes compiling and loading shaders.
    {
        auto& shader = passCtx.res.shader;

        ShaderPermutationKey key = {
            .shaderTag = "Lit",
            .passTag = "Forward", };

        shader = ctx->shaderManager->GetOrLoad(key);
         
        shader->SetStaticSampler("baseMapSampler", Samplers::LinearWrap(0));
        shader->SetStaticSampler("shadowMapSampler", Samplers::LinearClamp(1));
        shader->CreateRootSignature(); 

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
                MaterialDesc{ .shaderTag = "Lit" },
                RenderPassDesc{ .passTag = "Forward" },
                inputDescs
            );

    }
     
}

void Lit::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{ 
    Mesh::FlushAndRender(cmdList, passCtx); 

}

void Lit::BeginFrame(PassContext& passCtx) noexcept
{
     
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

    auto& cbAllocator = passCtx.res.cbAllocator;  
    auto& instAllocator = passCtx.res.instanceBufferAllocator;
     

    if (frameCtx->mainCamera) {

        SceneCB sceneCBData = {};
        sceneCBData.pvMatrix = frameCtx->mainCamera->pvMatrix; 
        passCtx.res.sceneCB->UploadData(&sceneCBData, sizeof(SceneCB));
    } 


    auto& fallbackTex = graphCtx->fallBackTexture;

    for (auto& proxy : frameCtx->staticMeshes) {

        auto currIndex = batchData.cmds.size();
        auto localHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());
         
        auto instBV = instAllocator->Upload(proxy.instanceData, proxy.instanceCount);  

        ObjectCB cb =
        {
            .modelMatrix = proxy.modelMatrix,
        }; 
        auto cbView = cbAllocator->Upload(&cb); 
        shader->SetCBV("ObjectCB", GetCBVDesc(cbView), localHeapOffset);  

        if (proxy.material->baseMap)
        {
            shader->SetSRV("baseMap", proxy.material->baseMap->GetRawResource(), proxy.material->baseMap->GetSRVDesc(), localHeapOffset);
        }
        else
        { 
            shader->SetSRV("baseMap", fallbackTex->GetRawResource(), fallbackTex->GetSRVDesc(), localHeapOffset);

        }
 
        if(graphCtx->shadowMap)
        shader->SetSRV("shadowMap", graphCtx->shadowMap->GetRawResource(), graphCtx->shadowMap->GetSRVDesc(), localHeapOffset);


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

void Lit::EndFrame(PassContext& passCtx) noexcept
{
    passCtx.data.Clear();

    //reset the allocator:
    passCtx.res.cbAllocator->Reset();
    passCtx.res.instanceBufferAllocator->Reset();
}

void Shadow::Init(const RendererContext* ctx, PassContext& passCtx)
{
    {
        auto& sceneCB = passCtx.res.sceneCB;
        sceneCB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
            .SizeInBytes = sizeof(SceneCB),
            .Format = DXGI_FORMAT_UNKNOWN, // Not used for constant buffers 
            .StrideInBytes = sizeof(SceneCB),
            .Usage = EBufferUsage::Upload | EBufferUsage::Constant
            }); 
    }


    {
        passCtx.res.cbAllocator
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
        shader->CreateRootSignature();

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

    auto& cbAllocator = passCtx.res.cbAllocator;
    auto& instAllocator = passCtx.res.instanceBufferAllocator;

    if (frameCtx->mainCamera) {

        SceneCB sceneCBData = {};
        sceneCBData.pvMatrix = frameCtx->mainCamera->pvMatrix; 
        passCtx.res.sceneCB->UploadData(&sceneCBData, sizeof(SceneCB));
    }

    for (auto& proxy : frameCtx->staticMeshes) {

        auto currIndex = batchData.cmds.size();
        auto localHeapOffset = static_cast<uint32_t>(baseHeapOffset.value() + currIndex * shader->GetDescriptorTableSize());
         
        auto instBV = instAllocator->Upload(proxy.instanceData, proxy.instanceCount);

        ObjectCB cb =
        {
            .modelMatrix = proxy.modelMatrix,
        };
        auto cbView = cbAllocator->Upload(&cb);
        shader->SetCBV("ObjectCB", GetCBVDesc(cbView), localHeapOffset); 
         

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

void Shadow::EndFrame(PassContext& passCtx) noexcept
{
    passCtx.data.Clear();

    //reset the allocator:
    passCtx.res.cbAllocator->Reset();
    passCtx.res.instanceBufferAllocator->Reset();
}

