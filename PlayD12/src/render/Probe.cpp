#include "PCH.h"
#include "Probe.h"

#include "Renderer.h"

void FProbe::CreateFromHDRI(SharedPtr<FD3D12Texture> equiRect)
{
    auto& ctx = Render::rendererContext;

    auto& rectDesc = equiRect->GetDesc(); 

    auto& desc = TexUtils::CreateCubeTextureDesc();
    desc.width = 256;
    desc.height = 256;
    desc.format = rectDesc.format;
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->envMap = CreateShared<FD3D12Texture>(ctx->device, desc);


    IBL::CubeMapCB cubeMapCB{
    .cubeSize = 256,
    };

    auto& cmd = rect2CubeCtx.cmd;
    cmd.groupX = (256 + 7) / 8;
    cmd.groupY = (256 + 7) / 8;
    cmd.groupZ = 6; 

    Compute::Init(ctx, rect2CubeCtx, "Post", "Rect2Cube");

    auto& shader = rect2CubeCtx.res.shader;
    auto& heapOffset = rect2CubeCtx.res.baseHeapOffset;
    shader->SetSRV("equirectHDR",
        equiRect->GetRawResource(),
        equiRect->GetSRVDesc(),
        heapOffset.value()
    );   

    shader->SetUAV("cubeMap",
        envMap->GetRawResource(),
        envMap->GetUAVDesc(),
        heapOffset.value()
    );

    //shader->SetCBV("cubeMapCB", 
    //    envMap->GetUAVDesc(),
    //    heapOffset.value()
    //);

    Compute::DispatchCompute(ctx->cmdList, rect2CubeCtx);
 
}
