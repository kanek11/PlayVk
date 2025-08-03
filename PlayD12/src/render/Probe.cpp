#include "PCH.h"
#include "Probe.h"

#include "Renderer.h"

void FProbe::Init()
{
    auto& ctx = Render::rendererContext;

    sharedCB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
      .SizeInBytes = sizeof(IBL::SharedCB),
      .Format = DXGI_FORMAT_UNKNOWN, // Not used for constant buffers 
      .StrideInBytes = sizeof(IBL::SharedCB),
      .Usage = EBufferUsage::Upload | EBufferUsage::Constant });

    IBL::SharedCB cb;
    sharedCB->UploadData(&cb, sizeof(IBL::SharedCB));

}


void FProbe::CreateFromHDRI(SharedPtr<FD3D12Texture> equiRect)
{
    auto& ctx = Render::rendererContext;

    auto& shader = rect2CubeCtx.res.shader;

    auto& rectDesc = equiRect->GetDesc();

    auto& desc = TexUtils::CreateCubeTextureDesc();
    desc.width = IBL::CUBE_FACE_SIZE;
    desc.height = IBL::CUBE_FACE_SIZE;
    desc.format = rectDesc.format;
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->envMap = CreateShared<FD3D12Texture>(ctx->device, desc);
 

    Compute::Init(ctx, rect2CubeCtx, "IBL", "Rect2Cube");

    //after init
    auto& heapOffset = rect2CubeCtx.res.baseHeapOffset;

    auto& cmd = rect2CubeCtx.cmd;
    cmd.groupX = (uint32_t)std::ceil(IBL::CUBE_FACE_SIZE / 8.0f);
    cmd.groupY = (uint32_t)std::ceil(IBL::CUBE_FACE_SIZE / 8.0f);
    cmd.groupZ = 6;
    cmd.heapOffset = heapOffset.value();


    shader->SetCBV("IBL_CB",
        sharedCB->GetCBVDesc(),
        heapOffset.value() 
    );

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



void FProbe::Finalize()
{
}

void FProbe::GenerateBRDFLUT()
{
    auto& ctx = Render::rendererContext;

    auto& brdfLUTCtx = this->brdfLUTCtx;
    auto& shader = brdfLUTCtx.res.shader; 


    auto& desc = FTextureDesc();
    desc.width  = IBL::BRDF_LUT_SIZE;
    desc.height = IBL::BRDF_LUT_SIZE;
    desc.format = DXGI_FORMAT_R16G16_FLOAT;
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->brdfLUT = CreateShared<FD3D12Texture>(ctx->device, desc);


    //
    Compute::Init(ctx, brdfLUTCtx, "IBL", "BRDFLUT");


    auto& heapOffset = brdfLUTCtx.res.baseHeapOffset;

    shader->SetCBV("IBL_CB",
        sharedCB->GetCBVDesc(),
        heapOffset.value()
    );

    shader->SetUAV("brdfLUT",
        brdfLUT->GetRawResource(),
        brdfLUT->GetUAVDesc(),
        heapOffset.value()
    );

    auto& cmd = brdfLUTCtx.cmd;
    cmd.groupX = (uint32_t)std::ceil(IBL::BRDF_LUT_SIZE / 8.0f);
    cmd.groupY = (uint32_t)std::ceil(IBL::BRDF_LUT_SIZE / 8.0f);
    cmd.groupZ = 1;
    cmd.heapOffset = heapOffset.value();

    Compute::DispatchCompute(ctx->cmdList, brdfLUTCtx);
}

void FProbe::GenerateIrradiance()
{ 
    auto& ctx = Render::rendererContext;

    auto& passCtx = this->irradianceCtx;
    auto& shader = passCtx.res.shader;

    auto& desc = TexUtils::CreateCubeTextureDesc();
    desc.width = IBL::IRRADIANCE_SIZE;
    desc.height = IBL::IRRADIANCE_SIZE;
    desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->diffuseIrradiance = CreateShared<FD3D12Texture>(ctx->device, desc);

    Compute::Init(ctx, passCtx, "IBL", "Irradiance");
     
    auto& heapOffset = passCtx.res.baseHeapOffset; 

    shader->SetCBV("IBL_CB",
        sharedCB->GetCBVDesc(),
        heapOffset.value()
    );

    shader->SetUAV("irradianceMap",
        diffuseIrradiance->GetRawResource(),
        diffuseIrradiance->GetUAVDesc(),
        heapOffset.value()
    );

    shader->SetSRV("envMap",
        envMap->GetRawResource(),
        envMap->GetSRVDesc(),
        heapOffset.value()
    );

    auto& cmd = passCtx.cmd;
    cmd.groupX = (uint32_t)std::ceil(IBL::IRRADIANCE_SIZE / 8.0f);
    cmd.groupY = (uint32_t)std::ceil(IBL::IRRADIANCE_SIZE / 8.0f);
    cmd.groupZ = 6;
    cmd.heapOffset = heapOffset.value();

    Compute::DispatchCompute(ctx->cmdList, passCtx);
}

void FProbe::GeneratePrefilter()
{
    auto& ctx = Render::rendererContext;
    auto& prefilterCtx = this->prefilterCtx;
    auto& shader = prefilterCtx.res.shader; 

    this->prefilterCBAllocator = CreateShared<FRingBufferAllocator<IBL::PrefilterCB>>(ctx->device, IBL::MAX_MIP);
        

    uint32_t baseRes= IBL::PREFILTER_SIZE;
    uint32_t numMips = IBL::MAX_MIP; //static_cast<uint32_t>(std::floor(std::log2(baseRes))) + 1;

    auto& desc = TexUtils::CreateCubeTextureDesc();
    desc.width = baseRes;
    desc.height = baseRes;
    desc.mipLevels = numMips; // don't forget!
    desc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.usages = ETextureUsage::UnorderedAccess | ETextureUsage::ShaderResource;

    this->specularPrefilter = CreateShared<FD3D12Texture>(ctx->device, desc);

    Compute::Init(ctx, prefilterCtx, "IBL", "Prefilter");
     
    auto& baseHeapOffset = prefilterCtx.res.baseHeapOffset; 

    for (uint32_t mip = 0; mip < numMips; ++mip)
    {
        uint32_t mipRes = baseRes >> mip;
        float roughness = (float)mip / (float)(numMips - 1);

        auto localHeapOffset = shader->RequestAllocationOnHeap();
            //baseHeapOffset.value(); //todo?

        IBL::PrefilterCB cb = { 
            .Resolution = mipRes,
            .NumSamples = 1024,
            .Roughness= roughness,
        };

        auto cbView = prefilterCBAllocator->Upload(&cb);  
        shader->SetCBV("prefilterCB", GetCBVDesc(cbView), localHeapOffset); 

        shader->SetCBV("IBL_CB", 
            sharedCB->GetCBVDesc(),
            localHeapOffset
        );
         
        // Bind the input environment map
        shader->SetSRV("envMap",
            envMap->GetRawResource(),
            envMap->GetSRVDesc(),
            localHeapOffset
        ); 


        // Set constant buffer with roughness and mip level  
        shader->SetUAV("prefilterMap",
            specularPrefilter->GetRawResource(),
            specularPrefilter->GetUAVDesc(mip),  //don't forget;  bug fix: the d slot is overwritten;
            localHeapOffset
        );

        // Dispatch compute shader for this mip
        auto& cmd = prefilterCtx.cmd;
        cmd.groupX = (uint32_t)std::ceil(mipRes/ 8.0f) ;
        cmd.groupY = (uint32_t)std::ceil(mipRes/ 8.0f) ;
        cmd.groupZ = 6;
        cmd.heapOffset = localHeapOffset;

        Compute::DispatchCompute(ctx->cmdList, prefilterCtx);

        //// Insert UAV barrier here to ensure UAV writes are finished before next dispatch 
        ctx->cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(specularPrefilter->GetRawResource()));

    }
}
