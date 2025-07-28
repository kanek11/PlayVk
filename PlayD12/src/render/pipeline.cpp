#include "PCH.h"    
#include "pipeline.h"


ComPtr<ID3D12PipelineState> PSOManager::GetOrCreate(
    const MaterialDesc& mat,
    const RenderPassDesc& pass,
    const std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc
) {
    // Step 1: Build shader permutation key
    ShaderPermutationKey shaderKey;
    shaderKey.shaderTag = mat.shaderTag;
    shaderKey.passTag = pass.passTag;
    //for (const auto& [k, v] : mat.keywords) {
    //    if (v) key.defines.insert(k);
    //}

    PSOKey psoKey{ shaderKey };
    if (cache.contains(psoKey))
    {
        std::cout << "PSO found in cache: " << psoKey.permutationKey.shaderTag << ", Pass: " << psoKey.permutationKey.passTag << '\n';
        return cache[psoKey].Get();
    }

    std::cout << "Creating new PSO for shader: " << psoKey.permutationKey.shaderTag << ", Pass: " << psoKey.permutationKey.passTag << '\n';

    // Step 2: Compile shader blobs
    auto shaderPerm = library.lock()->GetOrLoad(shaderKey);
    if (!shaderPerm) return nullptr;

    // Step 3: Build PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(shaderPerm->GetVSBlob().Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(shaderPerm->GetPSBlob().Get());


    // RootSignature: use default or per-shader-generated
   // extern ID3D12RootSignature* GetRootSignature(const std::string & shaderTag);
    //psoDesc.pRootSignature = GetRootSignature(mat.shaderTag);
    assert(shaderPerm->GetRootSignature() != nullptr);
    psoDesc.pRootSignature = shaderPerm->GetRootSignature().Get();


    psoDesc.InputLayout = { inputDesc.data(), static_cast<UINT>(inputDesc.size()) };
    psoDesc.PrimitiveTopologyType = pass.topology;

    // RenderTarget
    assert(pass.colorFormats.size() <= 8);
    psoDesc.NumRenderTargets = static_cast<UINT>(pass.colorFormats.size());
    for (size_t i = 0; i < pass.colorFormats.size(); ++i) {
        psoDesc.RTVFormats[i] = pass.colorFormats[i];
    }

    psoDesc.DSVFormat = pass.depthFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX; // No multisampling

    // Blend
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    if (mat.enableAlphaBlend) {
        auto& rt = psoDesc.BlendState.RenderTarget[0];
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D12_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D12_BLEND_ONE;
        rt.DestBlendAlpha = D3D12_BLEND_ZERO;
        rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Depth
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = pass.enableDepth ? TRUE : FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = mat.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

    // Rasterizer
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = pass.cullMode;
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;


    // Create PSO
    ComPtr<ID3D12PipelineState> pipelineState;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf())));

    cache[psoKey] = pipelineState;
    return pipelineState;
}

ComPtr<ID3D12PipelineState> PSOManager::GetOrCreateCompute(const MaterialDesc& mat, const RenderPassDesc& pass)
{ 
    ShaderPermutationKey shaderKey;
    shaderKey.shaderTag = mat.shaderTag;
    shaderKey.passTag = pass.passTag; 

    PSOKey psoKey{ shaderKey };
    if (cache.contains(psoKey))
    {
        std::cout << "PSO found in cache: " << psoKey.permutationKey.shaderTag << ", Pass: " << psoKey.permutationKey.passTag << '\n';
        return cache[psoKey].Get();
    }

    std::cout << "Creating new PSO for shader: " << psoKey.permutationKey.shaderTag << ", Pass: " << psoKey.permutationKey.passTag << '\n';
     
    auto shaderPerm = library.lock()->GetOrLoadCompute(shaderKey);
    if (!shaderPerm) return nullptr; 
    
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = shaderPerm->GetRootSignature().Get(); 
  
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(shaderPerm->GetCSBlob().Get());
    ComPtr<ID3D12PipelineState> computePSO;
    ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(computePSO.GetAddressOf())));

	cache[psoKey] = computePSO;
	return computePSO; 
}

 