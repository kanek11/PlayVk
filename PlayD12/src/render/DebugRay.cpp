#include "PCH.h"
#include "DebugRay.h"

#include "Renderer.h"

#include "Application.h"

using namespace Buffer;

void DebugDraw::Init(const RendererContext* ctx, PassContext& passCtx)
{
    auto& res = passCtx.res;

    size_t MaxVertices = 2 * MaxLines;


    {
        ShaderPermutationKey key = {
           Materials::DebugRayMaterialDesc.shaderTag,
           Passes::DebugRayPassDesc.passTag,
        };

        auto& shader = passCtx.res.shader;
        shader = ctx->shaderManager->GetOrLoad(key);


        passCtx.res.baseHeapOffset = shader->RequestAllocationOnHeap(MaxVertices);
    }


    {
        //input 
        auto inputDesc = InputLayoutBuilder::Build<Vertex>();

        auto& pso = passCtx.res.pso;
        // Create PSO for debug rendering
        pso = ctx->psoManager->GetOrCreate(
            Materials::DebugRayMaterialDesc,
            Passes::DebugRayPassDesc,
            inputDesc
        );

    }


    {
        auto& vb = passCtx.res.batchVB;
        // Create vertex buffer for debug lines
        vb = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
            sizeof(Vertex) * MaxVertices,
            EBufferUsage::Upload | EBufferUsage::Vertex
            });

        passCtx.res.batchVBV = Buffer::CreateBVStructured<Vertex>(vb);
    }


    {
        auto& mat = passCtx.debugCubeMat;
        mat->transparent = true;
    }

}

void DebugDraw::BeginFrame(PassContext& passCtx) noexcept
{
    //attach the debug mesh to transparent meshes:
    auto& frameData = Render::frameContext;

    auto& debugMeshes = passCtx.debugMeshes;

    frameData->transparentMeshes.insert(
        frameData->transparentMeshes.end(),
        debugMeshes.begin(),
        debugMeshes.end()
    );
}

void DebugDraw::EndFrame(PassContext& passCtx) noexcept
{
    //passCtx.data.ResetFrame();
}

void DebugDraw::FlushAndRender(ID3D12GraphicsCommandList* cmdList, const PassContext& passCtx) noexcept
{
    auto& res = passCtx.res;
    auto& data = passCtx.data;
    auto& shader = res.shader;

    auto& frameData = Render::frameContext;

    res.batchVB->UploadData(data.vertices.data(), data.vertices.size() * sizeof(Vertex));

    //
    cmdList->SetPipelineState(res.pso.Get());
    cmdList->SetGraphicsRootSignature(res.shader->GetRootSignature().Get());


    // Set the descriptor heap for the command list
    shader->SetDescriptorHeap(cmdList);
    shader->SetSceneRootCBV(
        cmdList,
        frameData->sceneCB->GetRawResource()
    );

    //or, e.g., cmd->SetGraphicsRootConstantBufferView(...)  

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    cmdList->IASetVertexBuffers(0, 1, &MakeVBV(res.batchVBV));

    cmdList->DrawInstanced(static_cast<UINT>(data.vertices.size()), 1, 0, 0);

}


void DebugDraw::AddLine(const Float3& start, const Float3& end,
    const Float4& color0,
    std::optional<Float4> color1)
{
#if (defined(DEBUG) || defined(_DEBUG)) 
    auto renderer = GameApplication::GetInstance()->GetRenderer();

    Vertex vert0 = {
        .position = start,
        .color = color0,
    };
    Vertex vert1 = {
       .position = end,
       .color = color1.has_value() ? color1.value() : color0,
    };

    renderer->AddLine(vert0, vert1);
#endif
}

void DebugDraw::AddRay(const Float3& origin, const Float3& direction, const Float4& color0, std::optional<Float4> color1)
{
    AddLine(origin, origin + direction, color0, color1);
}

void DebugDraw::ClearFrame()
{
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    renderer->ClearDebugDraw();
}

void DebugDraw::AddCube(const Float3& center, float size, std::optional<Float4> color)
{
#if (defined(DEBUG) || defined(_DEBUG)) 
    auto renderer = GameApplication::GetInstance()->GetRenderer();

    auto& ctx = renderer->debugRayCtx;

    auto S = MMath::MatrixScaling(size, size, size);
    auto T = MMath::MatrixTranslation(center.x(), center.y(), center.z());

    Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();
    modelMatrix = MatrixMultiply(S, modelMatrix);
    modelMatrix = MatrixMultiply(T, modelMatrix);

    auto& mat = ctx.debugCubeMat;
    mat->transparent = true;
    if (color.has_value())
        mat->transparentColor = color.value();

    //new
    auto newProxy = FStaticMeshProxy();
    newProxy.modelMatrix = modelMatrix;
    newProxy.mesh = ctx.debugCubMesh.get();
    newProxy.material = ctx.debugCubeMat.get();
    newProxy.instanceData = ctx.debugCubeInstances.data();
    newProxy.instanceCount = ctx.debugCubeInstances.size();

    renderer->AddDebugMesh(newProxy);
#endif
}