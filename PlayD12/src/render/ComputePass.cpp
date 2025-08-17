
#include "PCH.h"

#include "ComputePass.h"
#include "Renderer.h"

void Compute::Init(const RendererContext* ctx,
    ComputeContext& passCtx,
    const std::string& shaderTag,
    const std::string& passTag
)
{
    // Create shader permutation for compute shader
    ShaderPermutationKey key = {
        .shaderTag = shaderTag,
        .passTag = passTag,
    };

    passCtx.res.shader = ctx->shaderManager->GetOrLoadCompute(key);
    //passCtx.res.shader->CreateRootSignature();

    //// Create PSO for compute shader
    passCtx.res.pso = ctx->psoManager->GetOrCreateCompute(
        MaterialDesc{ .shaderTag = shaderTag },
        RenderPassDesc{ .passTag = passTag }
    );

    passCtx.res.baseHeapOffset = passCtx.res.shader->RequestAllocationOnHeap();

}

void Compute::DispatchCompute(ID3D12GraphicsCommandList* cmdList, const ComputeContext& ctx) noexcept
{
    //if (!passCtx.res.pso) return;
    cmdList->SetPipelineState(ctx.res.pso.Get());
    cmdList->SetComputeRootSignature(ctx.res.shader->GetRootSignature().Get());
    ctx.res.shader->SetDescriptorHeap(cmdList);
    //ctx.res.shader->SetDescriptorTablesCompute(cmdList, ctx.res.baseHeapOffset.value());
    ctx.res.shader->SetDescriptorTablesCompute(cmdList, ctx.cmd.heapOffset);
    //todo: set table;
    // Set the UAV for the compute shader 

    cmdList->Dispatch(ctx.cmd.groupX, ctx.cmd.groupY, ctx.cmd.groupZ);
}
