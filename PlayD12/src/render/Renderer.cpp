#include "PCH.h"
#include "Renderer.h" 

#include "Application.h"
using namespace DirectX;

D3D12HelloRenderer::D3D12HelloRenderer(UINT width, UINT height, std::wstring name,
    SharedPtr<WindowBase> mainWindow
)
{ 
    m_width = width;
    m_height = height;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    m_mainWindow = mainWindow;
}

void D3D12HelloRenderer::OnInit()
{  
    LoadPipeline();

    LoadAssets();

    //
    RenderPassInitContext ctx =
    {
      .device = m_device.Get(), 
      .cmdAllocator = m_commandAllocator.Get(),
      .cmdList = m_commandList.Get(),
      .cmdQueue = m_commandQueue.Get(),
      .m_shaderManager = this->m_shaderManager,
      .m_psoManager = this->m_psoManager
    };

    uiRenderer = CreateShared<UIRenderer>();
    uiRenderer->Init(ctx);


    InitShadowPass();
    InitRenderPass();


    DebugDraw::Get().Init(ctx);
    //m_debugRenderer = CreateShared<DebugRenderer>();
    //m_debugRenderer->Init(ctx);

    Render::rendererContext = new RendererFactoryContext{
        .device = m_device.Get(),
        .mainShaderPerm = m_shaderPerm,
        .shadowShaderPerm = m_shadowShaderPerm,
        .fallBackTexture = m_fallBackTexture,
    };

}

// Render the scene.
void D3D12HelloRenderer::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    //PopulateCommandList();
    BeginFrame();

     
    BeginShadowPass(m_commandList.Get());
    RecordShadowPassCommands(m_commandList.Get());
    EndShadowPass(m_commandList.Get());
    

    //
    BeginRenderPass(m_commandList.Get());

    RecordRenderPassCommands(m_commandList.Get());
     
    DebugDraw::Get().FlushAndRender(m_commandList.Get());

    uiRenderer->FlushAndRender(m_commandList.Get());

    EndRenderPass(m_commandList.Get());


    EndFrame();
}


void DebugDraw::AddRay(const Float3& origin, const Float3& direction,
    const Float4& color0,
    const Float4& color1
)
{

    this->AddLine(origin, origin + direction, color0, color1);

}

void DebugDraw::AddLine(const Float3& start, const Float3& end,
    const Float4& color0,
    const Float4& color1)
{
#if defined(_DEBUG)
    DebugLineVertex vert0 = {
        .position = start,
        .color = color0,
    };
    DebugLineVertex vert1 = {
       .position = end,
       .color = color1,
    };

    m_lineData.push_back(vert0);
    m_lineData.push_back(vert1);
#endif
}

void DebugDraw::FlushAndRender(ID3D12GraphicsCommandList* cmdList)
{
    if (m_lineData.empty()) return;

    //std::cout << "flush debug ray num: " << m_lineData.size() /2 << '\n';

    //auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(1280), static_cast<float>(720));
    //auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(1280), static_cast<LONG>(720));

    //
    cmdList->SetPipelineState(m_PSO.Get());
    cmdList->SetGraphicsRootSignature(m_shader->GetRootSignature().Get());

    //cmdList->RSSetViewports(1, &viewport);
    //cmdList->RSSetScissorRects(1, &scissorRect);


    // Set the descriptor heap for the command list
    m_shader->SetDescriptorHeap(cmdList);
    m_shader->SetDescriptorTables(cmdList, heapOffset);
    //or, e.g., cmd->SetGraphicsRootConstantBufferView(...)  

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());

    cmdList->DrawInstanced(static_cast<UINT>(m_lineData.size()), 1, 0, 0);

    m_lineData.clear();
}

DebugDraw& DebugDraw::Get()
{
    static DebugDraw instance;
    return instance;
}

void DebugDraw::Init(RenderPassInitContext ctx)
{
    size_t MaxVertices = 2 * MaxLines;

    // Create vertex buffer for debug lines
    m_vertexBuffer = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
        sizeof(DebugLineVertex) * MaxVertices,
        DXGI_FORMAT_UNKNOWN,
        sizeof(DebugLineVertex),
        EBufferUsage::Upload | EBufferUsage::Vertex
        });

    // Reserve CPU space for lines
    m_lineData.reserve(MaxVertices);

     
    //shader perm:
    ShaderPermutationKey key = {
        .shaderTag = "Debug",
        .passTag = "Line",
    };
    m_shader = ctx.m_shaderManager->GetOrLoad(key);
    m_shader->CreateRootSignature(); 

    //input layer:
    std::vector<VertexInputLayer> inputLayers =
    {
        VertexInputLayer{
            .slot = 0,
            .classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            .elements = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DebugLineVertex, position), sizeof(Float3), D3D12_APPEND_ALIGNED_ELEMENT },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(DebugLineVertex, color), sizeof(Float4), D3D12_APPEND_ALIGNED_ELEMENT }
            }
        }
    };

    // Create PSO for debug rendering
    m_PSO = ctx.m_psoManager->GetOrCreate(
        m_materialDesc,
        m_renderPassDesc,
        inputLayers
    );
     

    //the cosntant buffer:
    m_CB = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
        sizeof(MVPConstantBuffer),
        DXGI_FORMAT_UNKNOWN, // Not used for constant buffers
        256, // Alignment
        EBufferUsage::Upload | EBufferUsage::Constant
        });

    MVPConstantBuffer cbData = {};
    cbData.modelMatrix = MMath::MatrixIdentity<float, 4>();
    m_CB->UploadData(&cbData, sizeof(MVPConstantBuffer));

    //set CBV: 
    heapOffset = m_shader->RequestAllocationOnHeap();
    m_shader->SetCBV("MVPConstantBuffer",
        m_CB->GetRawResource(),
        m_CB->GetCBVDesc(),
        heapOffset);

}

void DebugDraw::OnUpdate(float delta, const Float4x4& pv)
{
    // Upload the constant buffer data
    MVPConstantBuffer cbData = {};
    cbData.projectionViewMatrix = pv;
    m_CB->UploadData(&cbData, sizeof(MVPConstantBuffer));

    m_vertexBuffer->UploadData(m_lineData.data(), m_lineData.size() * sizeof(DebugLineVertex));
}





// Load the rendering pipeline dependencies.
void D3D12HelloRenderer::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }

    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf())));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.GetAddressOf())));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(m_device.GetAddressOf())
        ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(m_device.GetAddressOf())
        ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf())));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        //Win32Application::GetHwnd(),
        (HWND)m_mainWindow->GetRawHandle(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(
        //Win32Application::GetHwnd(),
        (HWND)m_mainWindow->GetRawHandle(),
        DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        //D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        //rtvHeapDesc.NumDescriptors = FrameCount;
        //rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        //rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        //ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

        //m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        UINT numRTVDescriptors = FrameCount;
        m_SC_RTVHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, numRTVDescriptors);


        m_rangeHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, descriptorPoolSize);


        constexpr uint32_t rtvPoolSize = 32;
        m_rtvHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, rtvPoolSize);


        constexpr uint32_t dsvPoolSize = 16;
        m_dsvHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, dsvPoolSize);

    }

    // Create frame resources.
    {
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        //// Create a RTV for each frame.
        //for (UINT n = 0; n < FrameCount; n++)
        //{
        //    ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));
        //    m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        //    rtvHandle.Offset(1, m_rtvDescriptorSize);
        //}

        for (UINT n = 0; n < FrameCount; n++)
        {
            auto currIndex = m_SC_RTVHeapAllocator->Allocate(1); // Allocate one descriptor for this frame
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_SC_RTVHeapAllocator->GetCPUHandle(currIndex);

            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);

            //rtvHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); 
            m_bindings.rtvs.push_back(rtvHandle); // Store RTV handles for later use
        }
    }

    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
    }
 }

void D3D12HelloRenderer::LoadAssets()
{ 
    // Create the command list . after device;
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_PSO.Get(), IID_PPV_ARGS(m_commandList.GetAddressOf())));
     
    //close right after
    ThrowIfFailed(m_commandList->Close());

    m_shaderManager = CreateShared<ShaderLibrary>(m_device, m_rangeHeapAllocator);

    m_psoManager = CreateShared<PSOManager>(m_device, m_shaderManager);
}

void D3D12HelloRenderer::InitRenderPass()
{
    // Create the pipeline state, which includes compiling and loading shaders.
    {
        //todo: remove the sampler init to texture manager.
        D3D12_STATIC_SAMPLER_DESC sampler0 = {};
        sampler0.ShaderRegister = 0;
        sampler0.RegisterSpace = 0;
        sampler0.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        /*       sampler0.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
               sampler0.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
               sampler0.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;*/
        sampler0.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler0.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler0.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        sampler0.MipLODBias = 0;
        sampler0.MaxAnisotropy = 0;
        sampler0.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler0.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler0.MinLOD = 0.0f;
        sampler0.MaxLOD = D3D12_FLOAT32_MAX;
        sampler0.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


        D3D12_STATIC_SAMPLER_DESC sampler1 = {};
        sampler1.ShaderRegister = 1; // Shadow map sampler 
        sampler1.RegisterSpace = 0;
        sampler1.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler1.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler1.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler1.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler1.MipLODBias = 0;
        sampler1.MaxAnisotropy = 0;
        sampler1.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        //sampler1.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // Use opaque black for shadow map
        sampler1.MinLOD = 0.0f;
        sampler1.MaxLOD = D3D12_FLOAT32_MAX;
        sampler1.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


        ShaderPermutationKey key = {
            .shaderTag = "Lit",
            .passTag = "Forward", };

        auto shaderPerm = m_shaderManager->GetOrLoad(key);

        //todo: 
        shaderPerm->SetStaticSampler("baseMapSampler", sampler0);
        shaderPerm->SetStaticSampler("shadowMapSampler", sampler1); // if needed
        shaderPerm->CreateRootSignature();

        m_shaderPerm = shaderPerm;


        //------------
        //assemble input layout:
        auto& inputElementDescs = StaticMeshInputDesc::GetInputDescs();
        std::vector<VertexInputLayer> inputLayers;
        // Create a single input layer with all elements
        VertexInputLayer inputLayer;
        inputLayer.slot = 0; // Default slot
        inputLayer.classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // Default to per-vertex data
        inputLayer.elements = inputElementDescs;
        inputLayers.push_back(inputLayer);

        //the instance buffer:
        VertexInputLayer instanceLayer;
        instanceLayer.slot = 1;
        instanceLayer.classification = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        instanceLayer.instanceStepRate = 1;
        instanceLayer.elements.push_back({ "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 12 }); // 12 bytes for Float3
        inputLayers.push_back(instanceLayer);

        //------------

        m_PSO =
            m_psoManager->GetOrCreate(
                MaterialDesc{ .shaderTag = "Lit" },
                RenderPassDesc{ .passTag = "Forward" },
                inputLayers
            );

    }

    //depth related:
    {

        // Create a depth stencil texture.
        auto depthDesc = FTextureDesc{
            .width = m_width,
            .height = m_height,
            .format = DXGI_FORMAT_D32_FLOAT,
            .usages = { ETextureUsage::DepthStencil }
        };

        m_depthStencil = CreateShared<FD3D12Texture>(m_device.Get(), depthDesc);

        auto currIndex = m_dsvHeapAllocator->Allocate(1);
        auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(currIndex);

        m_device->CreateDepthStencilView(m_depthStencil->GetRawResource(), nullptr, dsvHandle);

        m_bindings.dsv = dsvHandle;
    }

    //teturing:
     // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    //me: so just declare it outside the local scope;  same scope with commandList:
    //ComPtr<ID3D12Resource> textureUploadHeap;

    //    // Create the command list . after device;
    //ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_PSO.Get(), IID_PPV_ARGS(m_commandList.GetAddressOf())));

    ThrowIfFailed(m_commandAllocator->Reset()); 
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

    // Create the texture.
    {
        auto texDesc = FTextureDesc{
            .width = TextureWidth,
            .height = TextureHeight,
            .format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .usages = { ETextureUsage::ShaderResource }
        };

        m_fallBackTexture = CreateShared<FD3D12Texture>(m_device.Get(), texDesc); 

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateFallBackTextureData();

        m_fallBackTexture->UploadFromCPU(
            m_commandList.Get(),
            texture.data(),
            TextureWidth * TexturePixelSize, // Row pitch,
            TextureWidth * TexturePixelSize * TextureHeight //slice pitch
        );
    }

    //new:
    { 
        auto atlasTex = uiRenderer->font->texture;
        auto atlasData = uiRenderer->font->imageData;
        atlasTex->UploadFromCPU(m_commandList.Get(), atlasData->data, 
            atlasData->metaInfo.rowPitch, 
            atlasData->metaInfo.slicePitch);

    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}



void D3D12HelloRenderer::InitShadowPass()
{
    //shader:
    {
        ShaderPermutationKey key = {
       .shaderTag = "Lit",
       .passTag = "Shadow", };

        this->m_shadowShaderPerm = m_shaderManager->GetOrLoad(key);
        this->m_shadowShaderPerm->CreateRootSignature();
    }

    //PSO
    {
        //assemble input layout:
        auto& inputElementDescs = StaticMeshInputDesc::GetInputDescs();
        std::vector<VertexInputLayer> inputLayers;
        // Create a single input layer with all elements
        VertexInputLayer inputLayer;
        inputLayer.slot = 0;
        inputLayer.classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // Default to per-vertex data
        // Only position for shadow pass
        inputLayer.elements = std::vector<FD3D12InputDesc>(inputElementDescs.begin(), inputElementDescs.begin() + 1);
        inputLayers.push_back(inputLayer);

        //------------

        m_shadowPSO =
            m_psoManager->GetOrCreate(
                shadowMaterialDesc,
                shadowPassDesc,
                inputLayers
            );

    }


    //--------- 
//attachments
    {

        auto shadowMapDesc = FTextureDesc{
    .width = m_shadowMapWidth,
    .height = m_shadowMapHeight,
    .format = DXGI_FORMAT_R32_TYPELESS, //D32_FLOAT
    .dsvFormat = DXGI_FORMAT_D32_FLOAT,
    .srvFormat = DXGI_FORMAT_R32_FLOAT,
    .usages = { ETextureUsage::DepthStencil, ETextureUsage::ShaderResource }
        };
        m_shadowMap = CreateShared<FD3D12Texture>(m_device.Get(), shadowMapDesc);

        //as dsv:
        auto dsvDesc = m_shadowMap->GetDSVDesc();

        auto currIndex = m_dsvHeapAllocator->Allocate(1);
        auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(currIndex);
        m_device->CreateDepthStencilView(m_shadowMap->GetRawResource(), &dsvDesc, dsvHandle);

        m_shadowBindings.dsv = dsvHandle;
    }
}

void D3D12HelloRenderer::BeginShadowPass(ID3D12GraphicsCommandList* commandList)
{

    // Bind only depth target  
    assert(m_shadowBindings.dsv.has_value());

    // Indicate that the shadow map will be used as a depth stencil target.
    auto srvToDsvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_shadowMap->GetRawResource(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

    commandList->ResourceBarrier(1, &srvToDsvBarrier);


    auto dsvHandle = m_shadowBindings.dsv.value();
    commandList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}

void D3D12HelloRenderer::EndShadowPass(ID3D12GraphicsCommandList* commandList)
{
    // barrier to SRV
// Transition the shadow map back to a shader resource state.
    auto dsvToSrvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_shadowMap->GetRawResource(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    commandList->ResourceBarrier(1, &dsvToSrvBarrier);
}

void D3D12HelloRenderer::RecordShadowPassCommands(ID3D12GraphicsCommandList* commandList)
{
    //
    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_shadowMapWidth), static_cast<float>(m_shadowMapHeight));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_shadowMapWidth), static_cast<LONG>(m_shadowMapHeight));

    // Set the pipeline state and root signature.
    commandList->SetPipelineState(m_shadowPSO.Get());
    commandList->SetGraphicsRootSignature(m_shadowShaderPerm->GetRootSignature().Get());

    // Set the viewport and scissor rectangle.
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);


    m_shadowShaderPerm->SetDescriptorHeap(m_commandList.Get());
    for (auto& proxy : m_staticMeshes) {
        m_shadowShaderPerm->SetDescriptorTables(m_commandList.Get(), proxy->shadowPassHeapOffset);

        m_commandList->IASetPrimitiveTopology(proxy->mesh->GetTopology());
        m_commandList->IASetVertexBuffers(0, 1, &proxy->mesh->GetVertexBuffer()->GetVertexBufferView());
        m_commandList->IASetIndexBuffer(&proxy->mesh->GetIndexBuffer()->GetIndexBufferView());

        auto indexCount = proxy->mesh->GetIndexCount();
        auto instanceCount = proxy->instanceProxy->instanceData.size();
        m_commandList->DrawIndexedInstanced((UINT)indexCount, (UINT)instanceCount, 0, 0, 0);
    }




}




void D3D12HelloRenderer::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}





void D3D12HelloRenderer::BeginFrame()
{
    //new: 
    if (meshDirty) {
        this->SetMeshDescriptors();
        meshDirty = false;
    }
    // Command list allocators can only be reset when the associated 
 // command lists have finished execution on the GPU; apps should use 
 // fences to determine GPU execution progress. 
     // However, when ExecuteCommandList() is called on a particular command 
 // list, that command list can then be reset at any time and must be before 
 // re-recording.
    ThrowIfFailed(m_commandAllocator->Reset());

    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

}

void D3D12HelloRenderer::EndFrame()
{
    ThrowIfFailed(m_commandList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void D3D12HelloRenderer::BeginRenderPass(ID3D12GraphicsCommandList* commandList)
{ 

    // Indicate that the back buffer will be used as a render target.
    auto ps_rtv_Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &ps_rtv_Barrier);


    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    //D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_SC_RTVHeapAllocator->GetCPUHandle(m_frameIndex);

    assert(m_bindings.rtvs.size() == FrameCount);
    auto rtvHandle = m_bindings.rtvs[m_frameIndex];  

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    //auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(0);  
    if (m_bindings.dsv.has_value()) {
        auto dsvHandle = m_bindings.dsv.value();
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    }

    else {
        m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    }

}



void D3D12HelloRenderer::RecordRenderPassCommands(ID3D12GraphicsCommandList* m_commandList)
{

    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    // 
   //begin---------------  
    m_commandList->SetPipelineState(m_PSO.Get());
    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_shaderPerm->GetRootSignature().Get());

    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);


    //record-------------
    m_shaderPerm->SetDescriptorHeap(m_commandList);

    //drawcall loop
    for (const auto& proxy : m_staticMeshes)
    {
        if (proxy->mesh == nullptr || proxy->mesh->GetVertexBuffer() == nullptr || proxy->mesh->GetIndexBuffer() == nullptr)
        {
            continue; // Skip if mesh is not valid
        }

        m_shaderPerm->SetDescriptorTables(m_commandList, proxy->mainPassHeapOffset);

        //m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetPrimitiveTopology(proxy->mesh->GetTopology());

        m_commandList->IASetVertexBuffers(0, 1, &proxy->mesh->GetVertexBuffer()->GetVertexBufferView());
        m_commandList->IASetVertexBuffers(1, 1, &proxy->instanceProxy->instanceBuffer->GetVertexBufferView());

        m_commandList->IASetIndexBuffer(&proxy->mesh->GetIndexBuffer()->GetIndexBufferView());

        //m_commandList->DrawInstanced(3, 1, 0, 0);
        //m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);  
        auto indiceCount = static_cast<UINT>(proxy->mesh->GetIndexCount());
        auto instanceCount = static_cast<UINT>(proxy->instanceProxy->instanceData.size());
        m_commandList->DrawIndexedInstanced(indiceCount, instanceCount, 0, 0, 0);
    }



}


void D3D12HelloRenderer::EndRenderPass(ID3D12GraphicsCommandList* commandList)
{ 
    //End------------
// Indicate that the back buffer will now be used to present.
    auto rtv_ps_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    m_commandList->ResourceBarrier(1, &rtv_ps_barrier);

}



void D3D12HelloRenderer::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}


_Use_decl_annotations_
void D3D12HelloRenderer::GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(factory6.GetAddressOf()))))
    {
        for (
            UINT adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(adapter.GetAddressOf()));
                ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    else
    {
        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}


// Generate a simple black and white checkerboard texture.
std::vector<UINT8> D3D12HelloRenderer::GenerateFallBackTextureData()
{
    const UINT size = 2;

    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT cellPitch = rowPitch >> size;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = TextureWidth >> size;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * TextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];


    for (UINT n = 0; n < textureSize; n += TexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}





void D3D12HelloRenderer::SetMeshDescriptors()
{
    std::cout << "set main pass descriptors" << std::endl;
    for (auto& mesh : m_staticMeshes)
    {
        m_shaderPerm->SetSRV("baseMap", mesh->material->baseMap->GetRawResource(), mesh->material->baseMap->GetSRVDesc(), mesh->mainPassHeapOffset);

        m_shaderPerm->SetSRV("shadowMap", m_shadowMap->GetRawResource(), m_shadowMap->GetSRVDesc(), mesh->mainPassHeapOffset);
        //m_shaderPerm->SetSRV("shadowMap", m_shadowMap->GetRawResource(), m_shadowMap->GetSRVDesc(), mesh->mainPassHeapOffset);

        auto constBufferRes = mesh->mainMVPConstantBuffer;
        m_shaderPerm->SetCBV("MVPConstantBuffer", constBufferRes->GetRawResource(), constBufferRes->GetCBVDesc(), mesh->mainPassHeapOffset);
    }

    std::cout << " set shadow pass descriptors" << std::endl;
    for (auto& mesh : m_staticMeshes)
    {
        auto constBufferRes = mesh->shadowMVPConstantBuffer;
        m_shadowShaderPerm->SetCBV("MVPConstantBuffer", constBufferRes->GetRawResource(), constBufferRes->GetCBVDesc(), mesh->shadowPassHeapOffset);
    }
}


// Update frame-based values.
void D3D12HelloRenderer::OnUpdate(float delta)
{

    //DebugDraw::Get().OnUpdate(delta, dummyCamera.pvMatrix);
    if (mainCamera) {
        DebugDraw::Get().OnUpdate(delta, mainCamera->pvMatrix);
    }
    else
    {
        //std::cout << "no camera" << '\n';
    }

}

void D3D12HelloRenderer::SubmitMesh(StaticMeshActorProxy* mesh)
{
    m_staticMeshes.push_back(mesh);
    meshDirty = true;
}

void D3D12HelloRenderer::ClearMesh()
{
    m_staticMeshes.clear();
}




void UIRenderer::Init(RenderPassInitContext ctx)
{ 
    //----------------------
    font = CreateShared<FontAtlas>();
    //font->LoadTexture("assets/ascii.png");
    //font->LoadTexture("assets/ASCII_full.png");
    font->LoadTexture("assets/ASCII_10x10.jpg");
    //font->LoadTexture("assets/test.png");
    //font->LoadTexture("assets/white.jpg");
     
    auto metaInfo = font->imageData.value().metaInfo;
    auto data = font->imageData.value().data;
    FTextureDesc atlasDesc =
        FTextureDesc{
        .width = static_cast<UINT>(metaInfo.width),
        .height = static_cast<UINT>(metaInfo.height),
        .format = metaInfo.format,
        .usages = {ETextureUsage::ShaderResource},
    };


    auto atlasTex = CreateShared<FD3D12Texture>(ctx.device, atlasDesc); 
    font->texture = atlasTex; 
    float cellWidth = (metaInfo.width -1)  / 10.0f;
    float cellHeight =  (metaInfo.height-1) / 10.0f;
    font->LoadGridAtlas(cellWidth, cellHeight, 10, 10);

    //font->LoadGridAtlas(80, 80, metaInfo.width ,metaInfo.height);

    D3D12_STATIC_SAMPLER_DESC sampler0 = {};
    sampler0.ShaderRegister = 0;
    sampler0.RegisterSpace = 0;
    sampler0.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; 
    sampler0.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler0.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler0.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; 
    sampler0.MipLODBias = 0;
    sampler0.MaxAnisotropy = 0;
    sampler0.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler0.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler0.MinLOD = 0.0f;
    sampler0.MaxLOD = D3D12_FLOAT32_MAX;
    sampler0.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; 


    //----------------------
    size_t MaxVertices = 4 * MaxUIBatch;
    size_t MaxIndices = 6 * MaxUIBatch;

    // Create vertex buffer for debug lines
    m_vertexBuffer = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
        sizeof(UIVertex) * MaxVertices,
        DXGI_FORMAT_UNKNOWN,
        sizeof(UIVertex),
        EBufferUsage::Upload | EBufferUsage::Vertex
        });


    m_indexBuffer = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
    sizeof(INDEX_FORMAT) * MaxIndices,
    DXGI_FORMAT_R16_UINT,
    sizeof(INDEX_FORMAT),
    EBufferUsage::Upload | EBufferUsage::Index
        });


    //shader perm:
    ShaderPermutationKey key = {
        .shaderTag = "UI",
        .passTag = "UI",
    };
    m_shader = ctx.m_shaderManager->GetOrLoad(key);

    m_shader->SetStaticSampler("fontAtlasSampler", sampler0);

    m_shader->CreateRootSignature();


    //input layer:
    std::vector<VertexInputLayer> inputLayers =
    {
        VertexInputLayer{
            .slot = 0,
            .classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            .elements = {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(UIVertex, position), sizeof(Float2), D3D12_APPEND_ALIGNED_ELEMENT },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(UIVertex, UV), sizeof(Float2), D3D12_APPEND_ALIGNED_ELEMENT },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(UIVertex, color), sizeof(Float4), D3D12_APPEND_ALIGNED_ELEMENT }, 
            }
        }
    };

    // Create PSO for debug rendering
    m_PSO = ctx.m_psoManager->GetOrCreate(
        m_materialDesc,
        m_renderPassDesc,
        inputLayers
    );
     
    m_CB = CreateShared<FD3D12Buffer>(ctx.device, FBufferDesc{
        sizeof(UISettingsCB),
        DXGI_FORMAT_UNKNOWN, // Not used for constant buffers
        sizeof(UISettingsCB),  
        EBufferUsage::Upload | EBufferUsage::Constant
        });

    UISettingsCB cbData = {};
    cbData.useTexture = true;
    m_CB->UploadData(&cbData, sizeof(UISettingsCB));

    //set CBV: 
    heapOffset = m_shader->RequestAllocationOnHeap(); 
     
    m_shader->SetCBV("UISettingsCB",
        m_CB->GetRawResource(),
        m_CB->GetCBVDesc(),
        heapOffset);

    m_shader->SetSRV("fontAtlas", atlasTex->GetRawResource(), atlasTex->GetSRVDesc(), heapOffset);
}
 


inline Float2 ScreenToNDC(int x, int y, int screenWidth, int screenHeight) {
    float ndcX = (2.0f * static_cast<float>(x + 0.5f) / static_cast<float>(screenWidth)) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(y + 0.5f) / static_cast<float>(screenHeight)); // Invert Y for top-left 
    //std::cout << "get ndc:" << ndcX << " " << ndcY << '\n';
    return { ndcX, ndcY };
}

void UIRenderer::AddQuad(const FRect& rect, const Float4& color)
{
    int screenWidth = GameApplication::GetInstance()->GetWidth();
    int screenHeight = GameApplication::GetInstance()->GetHeight();


    Float2 tl = ScreenToNDC(rect.x, rect.y, screenWidth, screenHeight);
    Float2 tr = ScreenToNDC(rect.x + rect.w, rect.y, screenWidth, screenHeight);
    Float2 bl = ScreenToNDC(rect.x, rect.y + rect.h, screenWidth, screenHeight);
    Float2 br = ScreenToNDC(rect.x + rect.w, rect.y + rect.h, screenWidth, screenHeight);

    uint32_t baseVertex = static_cast<uint32_t>(m_data.vertices.size());

    m_data.vertices.push_back({ tl, { 0.0f, 0.0f }, color }); // 0
    m_data.vertices.push_back({ tr, {1,0}, color }); // 1
    m_data.vertices.push_back({ bl, {0,1}, color }); // 2
    m_data.vertices.push_back({ br, {1,1}, color }); // 3


    uint32_t baseIndex = static_cast<uint32_t>(m_data.indices.size());

    m_data.indices.push_back(baseVertex + 0);
    m_data.indices.push_back(baseVertex + 1);
    m_data.indices.push_back(baseVertex + 2);

    m_data.indices.push_back(baseVertex + 2);
    m_data.indices.push_back(baseVertex + 1);
    m_data.indices.push_back(baseVertex + 3);

    UIDrawCmd cmd = {
    .indexOffset = baseIndex,
    .indexCount = 6,
    .useAtlas = true,
    };
    m_data.cmds.push_back(cmd);


    dirty = true;
}

void UIRenderer::AddQuad(const FRect& rect, const Float2& uvTL, const Float2& uvBR)
{
    int screenWidth = GameApplication::GetInstance()->GetWidth();
    int screenHeight = GameApplication::GetInstance()->GetHeight();
    Float2 tl = ScreenToNDC(rect.x, rect.y, screenWidth, screenHeight);
    Float2 tr = ScreenToNDC(rect.x + rect.w, rect.y, screenWidth, screenHeight);
    Float2 bl = ScreenToNDC(rect.x, rect.y + rect.h, screenWidth, screenHeight);
    Float2 br = ScreenToNDC(rect.x + rect.w, rect.y + rect.h, screenWidth, screenHeight);

    auto baseColor = Color::White;

    uint32_t baseVertex = static_cast<uint32_t>(m_data.vertices.size());
    m_data.vertices.push_back({ tl, uvTL, Color::White }); // 0
    m_data.vertices.push_back({ tr, {uvBR.x(), uvTL.y()}, Color::White }); // 1
    m_data.vertices.push_back({ bl, {uvTL.x(), uvBR.y()}, Color::White }); // 2
    m_data.vertices.push_back({ br, uvBR, Color::White }); // 3

    uint32_t baseIndex = static_cast<uint32_t>(m_data.indices.size());
    m_data.indices.push_back(baseVertex + 0);
    m_data.indices.push_back(baseVertex + 1);
    m_data.indices.push_back(baseVertex + 2);
    m_data.indices.push_back(baseVertex + 2);
    m_data.indices.push_back(baseVertex + 1);
    m_data.indices.push_back(baseVertex + 3);


    UIDrawCmd cmd = {
        .indexOffset = baseIndex,
        .indexCount = 6,
        .useAtlas = false,
    };
    m_data.cmds.push_back(cmd);
    dirty = true;
}

void UIRenderer::FlushAndRender(ID3D12GraphicsCommandList* cmdList)
{
    if (m_data.cmds.empty()) return;

    if (dirty) {
        m_vertexBuffer->UploadData(m_data.vertices.data(), m_data.vertices.size() * sizeof(UIVertex));
        m_indexBuffer->UploadData(m_data.indices.data(), m_data.indices.size() * sizeof(INDEX_FORMAT));
        dirty = false;
    }

    //std::cout << "flush UI draw cmd: " << m_data.cmds.size()  << '\n';

    //auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(1280), static_cast<float>(720));
    //auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(1280), static_cast<LONG>(720));

    //
    cmdList->SetPipelineState(m_PSO.Get());
    cmdList->SetGraphicsRootSignature(m_shader->GetRootSignature().Get());

    //cmdList->RSSetViewports(1, &viewport);
    //cmdList->RSSetScissorRects(1, &scissorRect); 

    // Set the descriptor heap for the command list
    m_shader->SetDescriptorHeap(cmdList);
    m_shader->SetDescriptorTables(cmdList, heapOffset);
    //or, e.g., cmd->SetGraphicsRootConstantBufferView(...)  

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    cmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
    cmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());

    for (auto& cmd : m_data.cmds) {
           
        auto indexCount = cmd.indexCount;
        auto indexOffset = cmd.indexOffset;
        cmdList->DrawIndexedInstanced((UINT)indexCount, 1, (UINT)indexOffset, 0, 0);
    }


    this->Clear();

    /*m_lineData.clear();*/
}