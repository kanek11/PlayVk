#include "PCH.h"
#include "Renderer.h" 

#include "Application.h"

#include "Loader.h"
#include "Texture.h"

#include "Asset.h"

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

void D3D12HelloRenderer::ConsumeCmdBuffer()
{
    //new: execute read buffer
    cmdBuffer.SwapBuffers();
    cmdBuffer.Execute();
}


//void D3D12HelloRenderer::SubmitCamera(const FCameraProxy& camera)
//{
//    sceneCBData.pvMatrix = camera.pvMatrix;
//    sceneCBData.invProj = camera.invProjMatrix;
//    sceneCBData.invView = camera.invViewMatrix;
//    sceneCBData.cameraPos = camera.position;
//}

void D3D12HelloRenderer::SubmitCamera(const FSceneView& sceneView)
{
    sceneCBData.pvMatrix = sceneView.pvMatrix;
    sceneCBData.invProj = sceneView.invProjMatrix;
    sceneCBData.invView = sceneView.invViewMatrix;
    sceneCBData.cameraPos = sceneView.position;
}


// Update frame-based values.  make sure comes before OnRender;
void D3D12HelloRenderer::OnUpdate(float delta)
{
    this->ConsumeCmdBuffer();

    //SceneCB sceneCBData = {};
    //if (this->mainCamera) {

    //    sceneCBData.pvMatrix = mainCamera->pvMatrix;
    //    sceneCBData.invProj = mainCamera->invProjMatrix;
    //    sceneCBData.invView = mainCamera->invViewMatrix;
    //    sceneCBData.cameraPos = mainCamera->position;
    //}
    //else {
    //    std::cerr << "no valid camera" << std::endl;
    //}

    //new:
    sceneCBData.OnTick();
    sceneCBData.deltaTime = delta;
    sceneCBData.viewportSize = { (float)m_width, (float)m_height };
    //this->sceneCB->UploadData(&sceneCBData, sizeof(SceneCB));
    
    this->sceneCB->UploadDataStructured(&sceneCBData);

    auto frameData = Render::frameContext;

    //frameData->mainCamera = mainCamera;
    //frameData->staticMeshes = staticMeshes;

    frameData->currentRTV = m_presentBindings.rtvs[m_frameIndex];

    frameData->sceneCB = this->sceneCB.get();

    //if (mainCamera) {
    //    DebugDraw::Get().OnUpdate(delta, mainCamera->pvMatrix); 

    //}
    //else
    //{
    //    //std::cout << "no camera" << '\n';
    //} 

}


void D3D12HelloRenderer::OnInit()
{
    LoadPipeline();

    LoadPipelineCommon();

    //
    Render::rendererContext = new RendererContext{
        .device = m_device.Get(),

        .cmdAllocator = m_commandAllocator.Get(),
        .cmdList = m_commandList.Get(),
        .cmdQueue = m_commandQueue.Get(),
        .shaderManager = this->m_shaderManager,
        .psoManager = this->m_psoManager,

        .dsvHeapAllocator = this->m_dsvHeapAllocator,
        .rtvHeapAllocator = this->m_rtvHeapAllocator,

    };
     

    //after system;
    LoadSystemResources();

    InitPresentPass();

    //for valid shadowmap;
    InitShadowPass();

    InitGBuffers();

    Render::frameContext = m_frame;
     
    Render::graphContext = m_graph;

    m_graph->shadowMapWidth = m_shadowMapWidth;
    m_graph->shadowMapHeight = m_shadowMapHeight;
    m_graph->shadowMap = m_shadowMap;
    m_graph->loadedTextures = this->loadTextures; 

    //for valid atlas
    UI::Init(Render::rendererContext, uiPassCtx);



    DebugMesh::Init(Render::rendererContext, debugMeshCtx);

    Shadow::Init(Render::rendererContext, shadowPassCtx);

    GBuffer::Init(Render::rendererContext, gbufferPassCtx);

    PBR::Init(Render::rendererContext, pbrShadingCtx);

    DebugDraw::Init(Render::rendererContext, debugRayCtx);
    //DebugDraw::Get().Init(Render::rendererContext);   

    InitEnvMap();

}

// Render the scene.
void D3D12HelloRenderer::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    //PopulateCommandList();  

    //GPU-side cmdList ; 
    //comes before the building of drawcmd;
    BeginFrame();

    //CPU-side
    //before the transparent pass
    DebugDraw::BeginFrame(debugRayCtx);
    DebugMesh::BeginFrame(debugMeshCtx);

    UI::BeginFrame(uiPassCtx);
    Shadow::BeginFrame(shadowPassCtx);
    GBuffer::BeginFrame(gbufferPassCtx);
    PBR::BeginFrame(pbrShadingCtx); 


    //rg->Execute(m_commandList.Get());

    //
    BeginShadowPass(m_commandList.Get());

    Shadow::FlushAndRender(m_commandList.Get(), shadowPassCtx);

    EndShadowPass(m_commandList.Get());

    //
    BeginGBufferPass(m_commandList.Get());

    GBuffer::FlushAndRender(m_commandList.Get(), gbufferPassCtx);

    EndGBufferPass(m_commandList.Get());
    

    //
    BeginPresentPass(m_commandList.Get()); 
     
    PBR::FlushAndRender(m_commandList.Get(), pbrShadingCtx);

    UI::FlushAndRender(m_commandList.Get(), uiPassCtx);

    DebugMesh::FlushAndRender(m_commandList.Get(), debugMeshCtx);

	DebugDraw::FlushAndRender(m_commandList.Get(), debugRayCtx);
    //DebugDraw::Get().FlushAndRender(m_commandList.Get());  

    EndPresentPass(m_commandList.Get());


    //
    EndFrame();

    // 
    UI::EndFrame(uiPassCtx);
    Shadow::EndFrame(shadowPassCtx);
    GBuffer::EndFrame(gbufferPassCtx);
    PBR::EndFrame(pbrShadingCtx);

    DebugMesh::EndFrame(debugMeshCtx);
	DebugDraw::EndFrame(debugRayCtx);
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

            ComPtr<ID3D12Debug1> debug1Controller;
            debugController.As< ID3D12Debug1>(&debug1Controller);
            debug1Controller->SetEnableGPUBasedValidation(true);
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

    //optional: cache the rtv:
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = swapChainDesc.Format;  //must match the swap chain format
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    this->sc_RTV = rtvDesc;


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


        constexpr uint32_t rtvPoolSize = 64;
        m_rtvHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, rtvPoolSize);


        constexpr uint32_t dsvPoolSize = 16;
        m_dsvHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, dsvPoolSize);

    }

    {
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
    }


}

void D3D12HelloRenderer::LoadPipelineCommon()
{
    // Create the command list . after device;
    {
        ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

        //close right after
        ThrowIfFailed(m_commandList->Close());
    }


    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));

        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

    }


    m_shaderManager = CreateShared<ShaderLibrary>(m_device, m_rangeHeapAllocator);

    m_psoManager = CreateShared<PSOManager>(m_device, m_shaderManager);


}

void D3D12HelloRenderer::LoadSystemResources()
{

    {
        //this->sceneCB = CreateShared<FD3D12Buffer>(m_device.Get(), FBufferDesc{
        //    .SizeInBytes = sizeof(SceneCB),
        //    .Format = DXGI_FORMAT_UNKNOWN, // Not used for constant buffers 
        //    .StrideInBytes = sizeof(SceneCB),
        //    .Usage = EBufferUsage::Upload | EBufferUsage::Constant
        //    });


        this->sceneCB = CreateShared<FD3D12Buffer>(m_device.Get(), FBufferDesc{
    .SizeInBytes = sizeof(SceneCB), 
    .Usage = EBufferUsage::Upload | EBufferUsage::Constant
            });

    }


    // Create the texture. 

    {
        auto texDesc = FTextureDesc{
            .width = TextureWidth,
            .height = TextureHeight,
            .format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .usages = { ETextureUsage::ShaderResource }
        };

        auto& tex = CreateShared<FD3D12Texture>(m_device.Get(), texDesc);
        loadTextures["Checkerboard"] = tex;

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        this->checkerBoardData = GenerateCheckerboardData();

        FUploadJob task = [=](ID3D12GraphicsCommandList* cmdList) {
            tex->UploadFromCPU(
                cmdList,
                this->checkerBoardData.data(),
                TextureWidth * TexturePixelSize, // Row pitch,
                TextureWidth * TexturePixelSize * TextureHeight //slice pitch
            );
            };

        this->UploadTexture({ task });
    }


    auto assetMgr = AssetManager::Get();
    assetMgr.LoadResources();
    auto& assets = assetMgr.textures;

    //bug fix: make sure the lambda context is valid, or just copy it.
    std::vector<FUploadJob> tasks;
    for (auto& [name, texData] : assets) {

        auto metaInfo = texData.metaInfo;
        FTextureDesc desc =
            FTextureDesc{
            .width = static_cast<UINT>(metaInfo.width),
            .height = static_cast<UINT>(metaInfo.height),
            .format = metaInfo.format,
            .usages = {ETextureUsage::ShaderResource},
        };

        auto& tex = CreateShared<FD3D12Texture>(m_device.Get(), desc);
        this->loadTextures[name] = tex;

        FUploadJob task = [=](ID3D12GraphicsCommandList* cmdList) {
            tex->UploadFromCPU(
                cmdList,
                texData.data,
                texData.metaInfo.rowPitch,
                texData.metaInfo.slicePitch
            );
            };

        tasks.push_back(task);

    }

    this->UploadTexture(tasks);

}

void D3D12HelloRenderer::InitPresentPass()
{
    {

        for (UINT n = 0; n < FrameCount; n++)
        {
            auto currIndex = m_SC_RTVHeapAllocator->Allocate(1); // Allocate one descriptor for this frame
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_SC_RTVHeapAllocator->GetCPUHandle(currIndex);

            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));

            //the rtv is derived;
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);

            //rtvHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); 
            m_presentBindings.rtvs.push_back(rtvHandle);
        }
    }


    {

        // Create a depth stencil texture.
        auto depthDesc = FTextureDesc{
    .width = m_width,
    .height = m_height,
    .format = DXGI_FORMAT_R32_TYPELESS, //D32_FLOAT can't be used directly as a shader resource 
    .dsvFormat = DXGI_FORMAT_D32_FLOAT,
    .srvFormat = DXGI_FORMAT_R32_FLOAT,
    .usages = ETextureUsage::DepthStencil | ETextureUsage::ShaderResource,
        };


        m_depthStencil = CreateShared<FD3D12Texture>(m_device.Get(), depthDesc);

        //as dsv:
        auto dsvDesc = m_depthStencil->GetDSVDesc();

        auto currIndex = m_dsvHeapAllocator->Allocate(1);
        auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(currIndex);

        m_device->CreateDepthStencilView(m_depthStencil->GetRawResource(), &dsvDesc, dsvHandle);

        m_presentBindings.dsv = dsvHandle;
        m_dsv = dsvHandle;
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
            .format = DXGI_FORMAT_R32_TYPELESS, //D32_FLOAT can't be used directly as a shader resource 
            .dsvFormat = DXGI_FORMAT_D32_FLOAT,
            .srvFormat = DXGI_FORMAT_R32_FLOAT,
            .usages = ETextureUsage::DepthStencil | ETextureUsage::ShaderResource,
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
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
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
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
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

    //donot use vsync
    //ThrowIfFailed(m_swapChain->Present(0, 0));

    WaitForPreviousFrame();


    //new:
    m_frame->clear();
}

void D3D12HelloRenderer::BeginPresentPass(ID3D12GraphicsCommandList* commandList)
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

    assert(m_presentBindings.rtvs.size() == FrameCount);
    auto rtvHandle = m_presentBindings.rtvs[m_frameIndex];

    const float clearColor[] = { 0.1f, 0.1f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    ////auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(0);  
    //if (m_presentBindings.dsv.has_value()) {
    //    auto dsvHandle = m_presentBindings.dsv.value();
    //    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); 

    //    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    //}

    //else {

    //    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    //}

    // //actually, the present pass is now pure post-processing, we don't depth;
    //todo: skip this by pass desc or sth.
    // 
    //m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    //new: we can continue using the gbuffer depth?
    if (m_gbufferAttachments.dsv.has_value())
    {
        //commandList->ClearDepthStencilView(m_gbufferAttachments.dsv.value(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_gbufferAttachments.dsv.value());
    }
    else {
        std::cerr << "GBuffer pass: DSV is not set!" << std::endl;
    }
     
    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);

}



void D3D12HelloRenderer::EndPresentPass(ID3D12GraphicsCommandList* commandList)
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
std::vector<UINT8> D3D12HelloRenderer::GenerateCheckerboardData()
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



//void D3D12HelloRenderer::SubmitMesh(StaticMeshActorProxy* meshProxy)
//{ 
//    Mesh::FStaticMeshProxy proxy = {
//    .modelMatrix = meshProxy->modelMatrix,
//    .mesh = meshProxy->mesh.get(),
//    .material = meshProxy->material.get(),
//    .instanceData = meshProxy->instanceData.data(),
//    .instanceCount = meshProxy->instanceData.size(),
//    };

    //staticMeshes.push_back(proxy);

    //litPassCtx.data.mainCamera = camera;
//}

void D3D12HelloRenderer::SubmitMesh(const FStaticMeshProxy& proxy)
{
    cmdBuffer.Enqueue([=] {
        if(!proxy.material->transparent)
        m_frame->staticMeshes.push_back(proxy);
        else
        m_frame->transparentMeshes.push_back(proxy);
        });
}

void D3D12HelloRenderer::SubmitMeshProxies(const std::vector<FStaticMeshProxy>& meshes)
{
    //cmdBuffer.Enqueue([=] {
    //    m_frame->staticMeshes.insert(
    //        m_frame->staticMeshes.end(),
    //        meshes.begin(),
    //        meshes.end()
    //    );
    //    });
    for (auto& mesh : meshes) {
        this->SubmitMesh(mesh);
    }
}

void D3D12HelloRenderer::ClearMesh()
{
    //staticMeshes.clear();
}




void D3D12HelloRenderer::AddQuad(const FQuadDesc& desc)
{
    //uiPassCtx.data.pendings.push_back(desc);    
    cmdBuffer.Enqueue([=] {
        uiPassCtx.data.pendings.push_back(desc);
        });
}

void D3D12HelloRenderer::AddQuad(const FRect& rect, const Float4& color)
{
    //delegate to  
    FQuadDesc desc = {
        .rect = rect,
        .uvTL = Float2(0.0f, 0.0f), 
        .uvBR = Float2(1.0f, 1.0f), 
        .color = color,
        .useAtlas = false,
    };
    AddQuad(desc);

}

void D3D12HelloRenderer::AddQuad(const FRect& rect, const Float2& uvTL, const Float2& uvBR)
{
    //delegate to
    FQuadDesc desc = {
        .rect = rect,
        .uvTL = uvTL,
        .uvBR = uvBR,
        .color = Float4(1.0f, 1.0f, 1.0f, 1.0f),
        .useAtlas = true,
    };

    AddQuad(desc);
}



void D3D12HelloRenderer::InitGBuffers()
{
    assert(Passes::GBufferPassDesc.rtvNames.size() == Passes::GBufferPassDesc.colorFormats.size());

    auto& names = Passes::GBufferPassDesc.rtvNames;
    int index{};
    for (auto& colorFormat : Passes::GBufferPassDesc.colorFormats)
    {
        FTextureDesc gbufferDesc = {
             .width = m_width,
             .height = m_height,
             .format = colorFormat,
             .usages = ETextureUsage::RenderTarget | ETextureUsage::ShaderResource,
        };
        auto gbufferTex = CreateShared<FD3D12Texture>(m_device.Get(), gbufferDesc);

        auto rtvHandle = AllocateCustomRT(gbufferTex.get());

        m_gbufferAttachments.rtvs.push_back(rtvHandle);

        auto& name = names[index++];
        m_graph->gbuffers[name] = gbufferTex;
    }

    // Create a depth stencil texture.
    auto depthDesc = FTextureDesc{
.width = m_width,
.height = m_height,
.format = DXGI_FORMAT_R32_TYPELESS, //D32_FLOAT can't be used directly as a shader resource 
.dsvFormat = DXGI_FORMAT_D32_FLOAT,
.srvFormat = DXGI_FORMAT_R32_FLOAT,
.usages = ETextureUsage::DepthStencil | ETextureUsage::ShaderResource,
    };

    auto depthMap = CreateShared<FD3D12Texture>(m_device.Get(), depthDesc);
    //as dsv:
    auto dsvDesc = depthMap->GetDSVDesc();
    auto currIndex = m_dsvHeapAllocator->Allocate(1);
    auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(currIndex);
    m_device->CreateDepthStencilView(depthMap->GetRawResource(), &dsvDesc, dsvHandle);

    m_graph->sceneDepth = depthMap;
    //depth reusing the system:
    m_gbufferAttachments.dsv = dsvHandle;

}

void D3D12HelloRenderer::BeginGBufferPass(ID3D12GraphicsCommandList* commandList)
{

    //transition the GBuffer textures to render target state. 
    for (const auto& [name, gbufferTex] : m_graph->gbuffers)
    {
        auto gbufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            gbufferTex->GetRawResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        commandList->ResourceBarrier(1, &gbufferBarrier);
    }


    //transit the depth:
    if (m_gbufferAttachments.dsv.has_value())
    {
        auto dsvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_graph->sceneDepth->GetRawResource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        );
        commandList->ResourceBarrier(1, &dsvBarrier);
    }
    else {
        std::cerr << "GBuffer pass: DSV is not set!" << std::endl;
    }


    //
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (const auto& rtv : m_gbufferAttachments.rtvs)
    {
        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }
    // Clear the depth stencil view.
    if (m_gbufferAttachments.dsv.has_value())
    {
        commandList->ClearDepthStencilView(m_gbufferAttachments.dsv.value(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        commandList->OMSetRenderTargets(static_cast<UINT>(m_gbufferAttachments.rtvs.size()), m_gbufferAttachments.rtvs.data(), FALSE, &m_gbufferAttachments.dsv.value());
    }
    else {
        std::cerr << "GBuffer pass: DSV is not set!" << std::endl;
    }

    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);

}

void D3D12HelloRenderer::EndGBufferPass(ID3D12GraphicsCommandList* commandList)
{

    // Transition the GBuffer textures back to shader resource state.
    for (const auto& [name, gbufferTex] : m_graph->gbuffers)
    {
        auto gbufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            gbufferTex->GetRawResource(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        );
        commandList->ResourceBarrier(1, &gbufferBarrier);
    }

    // Transition the depth texture back to shader resource state:
    if (m_gbufferAttachments.dsv.has_value())
    {
        auto dsvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_graph->sceneDepth->GetRawResource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        );
        commandList->ResourceBarrier(1, &dsvBarrier);
    }
    else {
        std::cerr << "GBuffer pass: DSV is not set!" << std::endl;
    }




}



void D3D12HelloRenderer::UploadTexture(std::vector<FUploadJob> tasks)
{
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

    {
        for (auto& task : tasks)
            task(m_commandList.Get());
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


    // wait until assets have been uploaded to the GPU.
    {
        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}



void D3D12HelloRenderer::AddLine(const DebugDraw::Vertex& vert0, const DebugDraw::Vertex& vert1)
{
	cmdBuffer.Enqueue([=] {
		debugRayCtx.data.vertices.push_back(vert0);
		debugRayCtx.data.vertices.push_back(vert1);
		});
}



void D3D12HelloRenderer::InitEnvMap()
{
    //auto& desc = TexUtils::CreateCubeTextureDesc();
    //desc.width = 256;
    //desc.height = 256;
    //desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //desc.usages = ETextureUsage::RenderTarget | ETextureUsage::ShaderResource;

    //m_graph->testCubeMap = CreateShared<FD3D12Texture>(m_device.Get(), desc);

    //UINT rowPitch = desc.width * 4;   // bytes per row
    //UINT slicePitch = rowPitch * desc.height; // bytes per face


    //UploadTask task = [=](ID3D12GraphicsCommandList* cmdList) {
    //    for (UINT face = 0; face < 6; ++face) {
    //        m_graph->testCubeMap->UploadFromCPU(m_commandList.Get(),
    //            checkerBoardData.data(),
    //            rowPitch, slicePitch,
    //            face);
    //    }
    //    };

    //this->UploadTexture({ task }); 
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));


    auto& probe = m_graph->probe;
    probe = CreateShared<FProbe>();

    auto& hdri = this->loadTextures["rogland_clear_night_1k.hdr"];
    //auto& hdri = this->loadTextures["autumn_field_puresky_1k.hdr"];
    //auto& hdri = this->loadTextures["citrus_orchard_road_puresky_1k.hdr"]; 
    //auto& hdri = this->loadTextures["kloppenheim_02_puresky_4k.hdr"]; 

    probe->Init();
    probe->CreateFromHDRI(hdri);
    probe->GenerateBRDFLUT();
    probe->GenerateIrradiance();
    probe->GeneratePrefilter();
    probe->Finalize();

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForPreviousFrame();

}