#include "PCH.h"
#include "ProcTex.h"

#include "Renderer.h"

void FPlayerTex::Init()
{
    //
    auto& renderCtx = Render::rendererContext;
    auto& graphCtx = Render::graphContext;

    //init tex
    auto& desc = FTextureDesc{}; 
    desc.width = cbData.Width;
    desc.height = cbData.Height; 
    desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;  
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->texture = CreateShared<FD3D12Texture>(renderCtx->device, desc);


    graphCtx->loadedTextures["PlayerFace"] = this->texture;


    //init ctx
    Compute::Init(renderCtx, this->computeCtx, "Custom", "Player");

    //cb if any


    //shader binding
    auto& shader = this->computeCtx.res.shader;
    auto& heapOffset = this->computeCtx.res.baseHeapOffset;

    shader->SetUAV("Output",
        this->texture->GetRawResource(),
        this->texture->GetUAVDesc(),
        heapOffset.value()
    );


    //dispatch cmd
    auto& cmd = this->computeCtx.cmd;
    cmd.groupX = (uint32_t)std::ceil(cbData.Height / 8.0f);
    cmd.groupY = (uint32_t)std::ceil(cbData.Width / 8.0f);
    cmd.groupZ = 1;
    cmd.heapOffset = heapOffset.value();

}

void FPlayerTex::Dispatch()
{
    auto& renderCtx = Render::rendererContext;
    Compute::DispatchCompute(renderCtx->cmdList, this->computeCtx);
}
