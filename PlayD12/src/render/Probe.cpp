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

    Compute::Init(ctx, rect2CubeCtx, "IBL", "Rect2Cube");

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

void FProbe::GenerateBRDFLUT()
{
	auto& ctx = Render::rendererContext; 

	auto& brdfLUTCtx = this->brdfLUTCtx; 
    auto& shader = brdfLUTCtx.res.shader;

    auto& desc = FTextureDesc();
	desc.width = 1024;
	desc.height = 1024;
	desc.format = DXGI_FORMAT_R16G16_FLOAT;
	desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

	this->brdfLUT = CreateShared<FD3D12Texture>(ctx->device, desc);

	Compute::Init(ctx, brdfLUTCtx, "IBL", "BRDFLUT");
	auto& heapOffset = brdfLUTCtx.res.baseHeapOffset;
	shader->SetUAV("brdfLUT",
		brdfLUT->GetRawResource(),
		brdfLUT->GetUAVDesc(),
		heapOffset.value()
	);

    auto& cmd = brdfLUTCtx.cmd;
    cmd.groupX = (1024 + 7) / 8;
    cmd.groupY = (1024 + 7) / 8;
	cmd.groupZ = 1; 

	Compute::DispatchCompute(ctx->cmdList, brdfLUTCtx);
}

void FProbe::GeneratePrefilter()
{
	auto& ctx = Render::rendererContext;
	auto& prefilterCtx = this->prefilterCtx;
	auto& shader = prefilterCtx.res.shader;


    //for (uint32_t mip = 0; mip < numMips; ++mip)
    //{
    //    uint res = baseResolution >> mip;
    //    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    //    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT
    //    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    //    uavDesc.Texture2DArray.MipSlice = mip;  //config miplevels
    //    uavDesc.Texture2DArray.FirstArraySlice = 0;
    //    uavDesc.Texture2DArray.ArraySize = 6; 
    //     
    //    shader->SetUAV("OutPrefilter", prefilterCube->GetRawResource(), uavDesc, heapOffset);
    //     
    //    cmd.groupX = (res + 7) / 8;
    //    cmd.groupY = (res + 7) / 8;
    //    cmd.groupZ = 6;
    //    Compute::DispatchCompute(cmdList, ctx);
    //}

}
