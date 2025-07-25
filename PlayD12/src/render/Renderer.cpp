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


// Update frame-based values.  make sure comes before OnRender;
void D3D12HelloRenderer::OnUpdate(float delta)
{ 
    auto frameData = Render::frameContext;

    frameData->mainCamera = mainCamera;
    frameData->staticMeshes = staticMeshes;

    //DebugDraw::Get().OnUpdate(delta, dummyCamera.pvMatrix);
    if (mainCamera) {
        DebugDraw::Get().OnUpdate(delta, mainCamera->pvMatrix); 

    }
    else
    {
        //std::cout << "no camera" << '\n';
    } 

}


void D3D12HelloRenderer::OnInit()
{
    LoadPipeline();

    LoadAssets();

    //
    Render::rendererContext = new RendererContext{
    .device = m_device.Get(),

    .cmdAllocator = m_commandAllocator.Get(),
    .cmdList = m_commandList.Get(),
    .cmdQueue = m_commandQueue.Get(),
    .shaderManager = this->m_shaderManager,
    .psoManager = this->m_psoManager,

    }; 
     

    //for valid atlas
    UI::Init(Render::rendererContext, uiPassCtx);

    //for valid fallback tex;
    InitRenderPass();

    //for valid shadowmap;
    InitShadowPass();

    Lit::Init(Render::rendererContext, litPassCtx);
    Shadow::Init(Render::rendererContext, shadowPassCtx); 

    DebugDraw::Get().Init(Render::rendererContext); 



    Render::frameContext = new FrameContext{};
    Render::graphContext = new RenderGraphContext{
        .shadowMapWidth = m_shadowMapWidth,
        .shadowMapHeight = m_shadowMapHeight,
        .shadowMap = m_shadowMap,

        .fallBackTexture = m_fallBackTexture,
    };

}

// Render the scene.
void D3D12HelloRenderer::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    //PopulateCommandList();  
    Lit::BeginFrame(litPassCtx);
    UI::BeginFrame(uiPassCtx);
    Shadow::BeginFrame(shadowPassCtx);

    BeginFrame(); 

    BeginShadowPass(m_commandList.Get());
    //RecordShadowPassCommands(m_commandList.Get());
    Shadow::FlushAndRender(m_commandList.Get(), shadowPassCtx);
    
    EndShadowPass(m_commandList.Get()); 
    
    //
    BeginRenderPass(m_commandList.Get());

    //RecordRenderPassCommands(m_commandList.Get());

    Lit::FlushAndRender(m_commandList.Get(), litPassCtx);

    //uiRenderer->FlushAndRender(m_commandList.Get());
    UI::FlushAndRender(m_commandList.Get(), uiPassCtx); 

    DebugDraw::Get().FlushAndRender(m_commandList.Get());


    EndRenderPass(m_commandList.Get());


    EndFrame();


    Lit::EndFrame(litPassCtx);
    UI::EndFrame(uiPassCtx);
    Shadow::EndFrame(shadowPassCtx);

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

void DebugDraw::Init(const RendererContext* ctx)
{
    size_t MaxVertices = 2 * MaxLines;

    // Create vertex buffer for debug lines
    m_vertexBuffer = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
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
    m_shader = ctx->shaderManager->GetOrLoad(key);
    m_shader->CreateRootSignature();

    //input 
    auto inputDesc = InputLayoutBuilder::Build<DebugLineVertex>();

    // Create PSO for debug rendering
    m_PSO = ctx->psoManager->GetOrCreate(
        m_materialDesc,
        m_renderPassDesc,
        inputDesc
    );


    //the cosntant buffer:
    m_CB = CreateShared<FD3D12Buffer>(ctx->device, FBufferDesc{
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
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

    //close right after
    ThrowIfFailed(m_commandList->Close());

    m_shaderManager = CreateShared<ShaderLibrary>(m_device, m_rangeHeapAllocator);

    m_psoManager = CreateShared<PSOManager>(m_device, m_shaderManager);
}

void D3D12HelloRenderer::InitRenderPass()
{
      
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
    if(uiPassCtx.data.font)
    {
        
        auto atlasTex = uiPassCtx.data.font->texture;
        assert(atlasTex != nullptr && "Font texture must be initialized before uploading data.");

        auto atlasData = uiPassCtx.data.font->imageData;
        atlasTex->UploadFromCPU(m_commandList.Get(), atlasData->data,
            atlasData->metaInfo.rowPitch,
            atlasData->metaInfo.slicePitch);

    }
    else
    {
        std::cerr << "atlas is not init" << std::endl;
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


     
    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_shadowMapWidth), static_cast<float>(m_shadowMapHeight));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_shadowMapWidth), static_cast<LONG>(m_shadowMapHeight));

    // Set the viewport and scissor rectangle.
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
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
 




void D3D12HelloRenderer::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}





void D3D12HelloRenderer::BeginFrame()
{ 
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


    //new:
    staticMeshes.clear();
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

    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));


    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);

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



void D3D12HelloRenderer::SubmitMesh(StaticMeshActorProxy* meshProxy)
{
    //m_staticMeshes.push_back(mesh);
    //meshDirty = true;     
  

    FStaticMeshProxy proxy = {
    .modelMatrix = meshProxy->modelMatrix,
    .mesh = meshProxy->mesh.get(), 
    .material = meshProxy->material.get(),
    .instanceData = meshProxy->instanceData.data(),
    .instanceCount = meshProxy->instanceData.size(),
    };
    staticMeshes.push_back(proxy);

    //litPassCtx.data.mainCamera = camera;
}

void D3D12HelloRenderer::ClearMesh()
{
    staticMeshes.clear();
}

void D3D12HelloRenderer::SubmitCamera(FCameraProxy* camera)
{ 
    mainCamera = camera; 
    //litPassCtx.data.mainCamera = camera;
}



void D3D12HelloRenderer::AddQuad(const UI::FQuadDesc& desc)
{

    uiPassCtx.data.pendings.push_back(desc);  
}

void D3D12HelloRenderer::AddQuad(const FRect& rect, const Float4& color)
{
    //delegate to  
    UI::FQuadDesc desc = {
        .rect = rect,
        .uvTL = Float2(0.0f, 0.0f), // default UV coordinates
        .uvBR = Float2(1.0f, 1.0f), // default UV coordinates
        .color = color,
        .useAtlas = false,
    };
    AddQuad(desc);

}

void D3D12HelloRenderer::AddQuad(const FRect& rect, const Float2& uvTL, const Float2& uvBR)
{

    //delegate to
    UI::FQuadDesc desc = {
        .rect = rect,
        .uvTL = uvTL,
        .uvBR = uvBR,
        .color = Float4(1.0f, 1.0f, 1.0f, 1.0f),
        .useAtlas = true,
    };

    AddQuad(desc);
}


