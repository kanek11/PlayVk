#include "PCH.h"
#include "Renderer.h" 

#include "Application.h"
using namespace DirectX;

constexpr int instanceCount = 1;
constexpr float spacing = 5.0f; // Adjust to control sparsity
constexpr float viewRadius = 20.0f; // Distance from the origin



D3D12HelloRenderer::D3D12HelloRenderer(UINT width, UINT height, std::wstring name,
    SharedPtr<WindowBase> mainWindow
) :
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{

    //new:
    m_width = width;
    m_height = height;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    m_mainWindow = mainWindow;
}

void D3D12HelloRenderer::OnInit()
{
    LoadPipeline();
    LoadAssets();
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
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


        constexpr uint32_t descriptorPoolSize = 100;
        m_rangeHeapAllocator = CreateShared<FDescriptorHeapAllocator>(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorPoolSize);

    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].GetAddressOf())));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
}

// Load the sample assets.
void D3D12HelloRenderer::LoadAssets()
{

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        //todo: remove the sampler init to texture manager.
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


        auto assetPathVS = GameApplication::GetInstance()->GetAssetFullPath("shaders/bin/hello_lighting_VS.cso");
        auto assetPathPS = GameApplication::GetInstance()->GetAssetFullPath("shaders/bin/hello_lighting_PS.cso");

        m_shaderManager = CreateShared<FD3D12GraphicsShaderManager>(m_device, m_rangeHeapAllocator);
        m_shaderManager->LoadShaders(assetPathVS, assetPathPS);
        m_shaderManager->PrepareRootSignature();

        m_shaderManager->SetStaticSampler("baseMapSampler", sampler);

        m_shaderManager->CreateRootSignature();

    }

    // Create root signature.  
    {
        auto& inputElementDescs = StaticMeshInputDesc::GetInputDescs();

        std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
        for (const auto& desc : inputElementDescs)
        {
            auto wideName = std::wstring(desc.semanticName.begin(), desc.semanticName.end());
            D3D12_INPUT_ELEMENT_DESC inputDesc = {};
            inputDesc.SemanticName = desc.semanticName.c_str();
            inputDesc.SemanticIndex = static_cast<UINT>(desc.semanticIndex);
            inputDesc.Format = desc.format;
            inputDesc.AlignedByteOffset = static_cast<UINT>(desc.alignedByteOffset);
            inputDesc.InputSlot = 0;
            inputDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // Default to per-vertex data
            inputDesc.InstanceDataStepRate = 0; // Default to 0 for per-vertex data

            inputDescs.push_back(inputDesc);

        };

        inputDescs.push_back({ "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        //psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.InputLayout = { inputDescs.data(), static_cast<UINT>(inputDescs.size()) };

        psoDesc.pRootSignature = m_shaderManager->GetRootSignature().Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_shaderManager->GetVSBlob().Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_shaderManager->GetPSBlob().Get());

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; //D3D12_CULL_MODE_BACK; // Can also be FRONT or NONE
        rasterizerDesc.FrontCounterClockwise = TRUE;   // TRUE means counter-clockwise is front

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
        //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        //psoDesc.DepthStencilState.DepthEnable = FALSE;
        //psoDesc.DepthStencilState.StencilEnable = FALSE; 
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf())));
    }




    //depth related:
    {
        D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT, // Or DXGI_FORMAT_D24_UNORM_S8_UINT if you need stencil
            m_width,
            m_height,
            1, 0, 1, 0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        );

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil = { 1.0f, 0 };

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(&m_depthStencil)
        ));


        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

        m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    }


    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(m_commandList.GetAddressOf())));


    //teturing:
     // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    //me: so just declare it outside the local scope;  same scope with commandList:
    ComPtr<ID3D12Resource> textureUploadHeap;

    // Create the texture.
    {
        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(m_fallBackTexture.GetAddressOf())));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_fallBackTexture.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(textureUploadHeap.GetAddressOf())));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateFallBackTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = TextureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources(m_commandList.Get(), m_fallBackTexture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_fallBackTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        this->m_fallBackSRVDesc = srvDesc;

        //m_shaderManager->SetSRV("baseMap", m_fallBackTexture, srvDesc, cubeHeapStartOffset); 
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);






    {
        InitMeshAssets();
        SetMeshDescriptors();
    }


    //new: init the instance data:
    {
    }


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



// Render the scene.
void D3D12HelloRenderer::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void D3D12HelloRenderer::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloRenderer::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_shaderManager->GetRootSignature().Get());


    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    //new: depth
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);


    m_shaderManager->SetDescriptorHeap(m_commandList);

    //drawcall loop
    for (const auto& proxy : m_staticMeshes)
    {
        if (proxy->mesh == nullptr || proxy->mesh->GetVertexBuffer() == nullptr || proxy->mesh->GetIndexBuffer() == nullptr)
        {
            continue; // Skip if mesh is not valid
        }

        m_shaderManager->SetDescriptorTables(m_commandList, proxy->heapStartOffset);

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


    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
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
    for (auto& mesh : m_staticMeshes)
    {
        m_shaderManager->SetSRV("baseMap", mesh->material->baseMapResource, mesh->material->baseMapSRV, mesh->heapStartOffset);

        auto constBufferRes = mesh->constantBuffer;
        m_shaderManager->SetCBV("SceneConstantBuffer", constBufferRes->GetResource(), constBufferRes->GetCBVDesc(), mesh->heapStartOffset);
    }
}

StaticMeshObjectProxy* D3D12HelloRenderer::InitMesh(SharedPtr<UStaticMesh> mesh,
    FLOAT3 position,
    FLOAT3 scale
)
{
    //------------------------------
    auto heapStartOffset = m_shaderManager->RequestAllocationOnHeap();
    std::cout << "Heap start offset for mesh: " << heapStartOffset << std::endl;


    //------------------------------
    SceneConstantBuffer constantBufferData{};

    auto constBuffer = CreateShared<FD3D12Buffer>(m_device.Get(), FBufferDesc{
        sizeof(SceneConstantBuffer),
        DXGI_FORMAT_UNKNOWN, // Not used for constant buffers
        256, // Alignment
        EBufferUsage::Upload | EBufferUsage::Constant
        });

    //update the constant buffer data:
    constBuffer->UploadData(&constantBufferData, sizeof(constantBufferData));


    //------------------------------
    auto instanceData = GenerateInstanceData();
    UINT instanceBufferSize = static_cast<UINT>(sizeof(InstanceData) * instanceData.size());

    auto instanceBuffer = CreateShared<FD3D12Buffer>(m_device.Get(), FBufferDesc{
        instanceBufferSize,
        DXGI_FORMAT_UNKNOWN, // Not used for instance buffers
        sizeof(InstanceData),
        EBufferUsage::Upload | EBufferUsage::Vertex
        });

    instanceBuffer->UploadData(instanceData.data(), instanceBufferSize);

    //------------------------------
    auto meshProxy = new StaticMeshObjectProxy{
        .position = position,
        .scale = scale,

        .mesh = mesh,
        .heapStartOffset = heapStartOffset,

        .material = CreateShared<FMaterialProxy>(m_fallBackTexture, m_fallBackSRVDesc),

        .constantBuffer = constBuffer,

        .instanceProxy = CreateShared<FInstanceProxy>(
            FInstanceProxy{
                .instanceData = instanceData,
                .instanceBuffer = instanceBuffer
            }
        )
    };

    return meshProxy;
}



void D3D12HelloRenderer::InitMeshAssets()
{
    auto physicsScene = GameApplication::GetInstance()->GetPhysicalScene();


    //auto cubeMesh0 = CreateShared<CubeMesh>();
    //cubeMesh0->CreateGPUResource(m_device.Get());
    //auto cubeMeshProxy0 = InitMesh(cubeMesh0,
    //    { -5.0f, 0.0f, 0.0f },
    //    { 1.0f, 1.0f, 1.0f }
    //);
    //
    ////new rigidbody:
    //auto rigidBody0 = new RigidBody(cubeMeshProxy0, cubeMeshProxy0->position, Box{ cubeMeshProxy0->scale });
    //cubeMeshProxy0->rigidBody = rigidBody0;
    //physicsScene->AddRigidBody(rigidBody0);
    //
    //auto boxCollider0 = new Collider(cubeMeshProxy0, Box{ cubeMeshProxy0->scale }, rigidBody0); //half extents
    //cubeMeshProxy0->collider = boxCollider0;
    //physicsScene->AddCollider(boxCollider0);
    //
    //rigidBody0->enableRotation = false; 

    auto cubeMesh1 = CreateShared<CubeMesh>();
    cubeMesh1->CreateGPUResource(m_device.Get());
    auto cubeMeshProxy1 = InitMesh(cubeMesh1,
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////a initial rotation of the cube: 
    //auto rotation1 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMConvertToRadians(20.0f));
    //cubeMeshProxy1->rotation = XMQuaternionMultiply(rotation1, cubeMeshProxy1->rotation);

    //new rigidbody for the cube:
    auto rigidBody1 = new RigidBody(cubeMeshProxy1, cubeMeshProxy1->position, Box{ cubeMeshProxy1->scale }, cubeMeshProxy1->rotation);
    cubeMeshProxy1->rigidBody = rigidBody1;
    physicsScene->AddRigidBody(rigidBody1); //add to the physics scene
    auto boxCollider1 = new Collider(cubeMeshProxy1, Box{ cubeMeshProxy1->scale }, rigidBody1); //half extents
    cubeMeshProxy1->collider = boxCollider1;
    physicsScene->AddCollider(boxCollider1); //add to the physics scene
    
    //set to static:
    rigidBody1->mass = MMath::FLOAT_MAX; //set the mass to 0, so it is static
    rigidBody1->simulatePhysics = false; //make the cube static, not affected by physics
    
    rigidBody1->simulateRotation = false; //make the cube not affected by rotation, so it will not fall down due to gravity
    
    rigidBody1->debugName = "cubeStatic0";



    //on top of cube1;
    auto cubeMesh2 = CreateShared<CubeMesh>();
    cubeMesh2->CreateGPUResource(m_device.Get());
    auto cubeMeshProxy2 = InitMesh(cubeMesh2,
        { -1.0f, 4.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////a initial rotation of the cube: 
    auto rotation2 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(30.0f));
    cubeMeshProxy2->rotation = XMQuaternionMultiply(rotation2, cubeMeshProxy2->rotation);

    ////new rigidbody for the cube:
    auto rigidBody2 = new RigidBody(cubeMeshProxy2, cubeMeshProxy2->position, Box{ cubeMeshProxy2->scale }, cubeMeshProxy2->rotation);
    cubeMeshProxy2->rigidBody = rigidBody2;
    physicsScene->AddRigidBody(rigidBody2);
    auto boxCollider2 = new Collider(cubeMeshProxy2, Box{ cubeMeshProxy2->scale }, rigidBody2); //half extents
    cubeMeshProxy2->collider = boxCollider2;
    physicsScene->AddCollider(boxCollider2);

    rigidBody2->simulateRotation = true;
    rigidBody2->debugName = "cubeDrop1";


    //auto cubeMesh3 = CreateShared<CubeMesh>();
    //cubeMesh3->CreateGPUResource(m_device.Get());
    //
    //auto cubeMeshProxy3 = InitMesh(cubeMesh3,
    //    { 0.0f, 0.0f, 9.0f },
    //    { 1.0f, 1.0f, 1.0f }
    //);


    //new rigidbody for the cube:
    //auto rigidBody3 = new RigidBody(cubeMeshProxy3, cubeMeshProxy3->position, Box{ cubeMeshProxy3->scale });
    //cubeMeshProxy3->rigidBody = rigidBody3;
    //physicsScene->AddRigidBody(rigidBody3); //add to the physics scene
    //auto boxCollider3 = new Collider(cubeMeshProxy3, Box{ cubeMeshProxy3->scale }, rigidBody3); //half extents
    //cubeMeshProxy3->collider = boxCollider3;
    //physicsScene->AddCollider(boxCollider3); //add to the physics scene
    //
    //
    //rigidBody3->enableRotation = false;
    // 
     
    //cube4：
    auto cubeMesh4 = CreateShared<CubeMesh>();
    cubeMesh4->CreateGPUResource(m_device.Get());
        
    //auto cubeMeshProxy4 = InitMesh(cubeMesh4,
    //    { -1.0f, 8.0f, 0.0f },
    //    { 1.0f, 1.0f, 1.0f }
    //);
    ////new rigidbody for the cube:
    //auto rigidBody4 = new RigidBody(cubeMeshProxy4, cubeMeshProxy4->position, Box{ cubeMeshProxy4->scale }, cubeMeshProxy4->rotation);
    //cubeMeshProxy4->rigidBody = rigidBody4;
    //physicsScene->AddRigidBody(rigidBody4); //add to the physics scene
    //auto boxCollider4 = new Collider(cubeMeshProxy4, Box{ cubeMeshProxy4->scale }, rigidBody4); //half extents
    //cubeMeshProxy4->collider = boxCollider4;
    //physicsScene->AddCollider(boxCollider4); //add to the physics scene
    //    
    //rigidBody4->simulateRotation = false;
    //rigidBody4->debugName = "dropCube2";


        //auto cubeMesh5 = CreateShared<CubeMesh>();
        //cubeMesh5->CreateGPUResource(m_device.Get());
        //auto cubeMeshProxy5 = InitMesh(cubeMesh5,
        //    { 2.0f, -15.0f, 0.0f },
        //    { 20.0f, 10.0f, 20.0f }
        //);

        ////new rigidbody for the cube:
        //auto rigidBody5 = new RigidBody(cubeMeshProxy5, cubeMeshProxy5->position, Box{ cubeMeshProxy5->scale });
        //cubeMeshProxy5->rigidBody = rigidBody5;
        //physicsScene->AddRigidBody(rigidBody5); //add to the physics scene
        //auto boxCollider5 = new Collider(cubeMeshProxy5, Box{ cubeMeshProxy5->scale }, rigidBody5); //half extents
        //cubeMeshProxy5->collider = boxCollider5;
        //physicsScene->AddCollider(boxCollider5); //add to the physics scene 
        // 
        //rigidBody5->mass = MMath::FLOAT_MAX; //set the mass to 0, so it is static
        //rigidBody5->simulatePhysics = false; //make the cube static, not affected by physics

        //rigidBody5->enableRotation = false; //make the cube not affected by rotation, so it will not fall down due to gravity


    auto sphereMesh0 = CreateShared<SphereMesh>();
    sphereMesh0->CreateGPUResource(m_device.Get());
    auto sphereMeshProxy0 = InitMesh(sphereMesh0,
        { 1.5f, 10.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    );

    ////new rigidbody for the sphere:
    auto rigidBodySphere0 = new RigidBody(sphereMeshProxy0, sphereMeshProxy0->position, Sphere{ sphereMeshProxy0->scale.x() }, sphereMeshProxy0->rotation);
    sphereMeshProxy0->rigidBody = rigidBodySphere0;
    physicsScene->AddRigidBody(rigidBodySphere0); //add to the physics scene
    auto sphereCollider0 = new Collider(sphereMeshProxy0, Sphere{ sphereMeshProxy0->scale.x() }, rigidBodySphere0); //sphere radius
    sphereMeshProxy0->collider = sphereCollider0;
    physicsScene->AddCollider(sphereCollider0); //add to the physics scene 
    
    rigidBodySphere0->debugName = "Sphere0";  
    rigidBodySphere0->simulateRotation = true; //make the sphere affected by rotation, so it will fall down due to gravity


    //auto sphereMesh1 = CreateShared<SphereMesh>();
    //sphereMesh1->CreateGPUResource(m_device.Get());
    //auto sphereMeshProxy1 = InitMesh(sphereMesh1,
    //    { 0.0f, 0.0f, -3.0f },
    //    { 1.0f, 1.0f, 1.0f }
    //);
    //
    ////new rigidbody for the sphere:
    //auto rigidBodySphere1 = new RigidBody(sphereMeshProxy1, sphereMeshProxy1->position, Sphere{});
    //sphereMeshProxy1->rigidBody = rigidBodySphere1;
    //physicsScene->AddRigidBody(rigidBodySphere1); //add to the physics scene
    //auto sphereCollider1 = new Collider(sphereMeshProxy1, Sphere{ sphereMeshProxy1->scale.x() }, rigidBodySphere1); //sphere radius
    //sphereMeshProxy1->collider = sphereCollider1;
    //physicsScene->AddCollider(sphereCollider1); //add to the physics scene
    //
    ////set to static:
    //rigidBodySphere1->mass = MMath::FLOAT_MAX; //set the mass to 0, so it is static
    //rigidBodySphere1->simulatePhysics = false; //make the sphere static, not affected by physics
    //
    //rigidBodySphere1->debugName = "Sphere1";
    //rigidBodySphere1->enableRotation = false; //make the sphere not affected by rotation, so it will not fall down due to gravity
    //
    //sphere2:
    //auto sphereMesh2 = CreateShared<SphereMesh>();
    //sphereMesh2->CreateGPUResource(m_device.Get());
    //auto sphereMeshProxy2 = InitMesh(sphereMesh2,
    //    { 0.0f, 4.0f, -3.7f },
    //    { 1.0f, 1.0f, 1.0f }
    //);

    //new rigidbody for the sphere:
    //auto rigidBodySphere2 = new RigidBody(sphereMeshProxy2, sphereMeshProxy2->position, Sphere{ sphereMeshProxy2->scale.x() }, sphereMeshProxy2->rotation);
 //   sphereMeshProxy2->rigidBody = rigidBodySphere2;
 //   physicsScene->AddRigidBody(rigidBodySphere2); //add to the physics scene
 //   auto sphereCollider2 = new Collider(sphereMeshProxy2, Sphere{ sphereMeshProxy2->scale.x() }, rigidBodySphere2); //sphere radius
 //   sphereMeshProxy2->collider = sphereCollider2;
 //   physicsScene->AddCollider(sphereCollider2); //add to the physics scene 
 //   
 //   rigidBodySphere2->debugName = "Sphere2"; 
    //rigidBodySphere2->enableRotation = true; //make the sphere affected by rotation, so it will fall down due to gravity


    auto plane0 = CreateShared<PlaneMesh>();
    plane0->CreateGPUResource(m_device.Get());
    auto planeMeshProxy = InitMesh(plane0,
        { 0.0f, -4.0f, 0.0f },
        { 30.0f, 30.0f, 30.0f }
    );

    //add a rigidbody for the plane:
    auto rigidBodyPlane = new RigidBody(planeMeshProxy, planeMeshProxy->position, Plane{ planeMeshProxy->scale.x(), planeMeshProxy->scale.y() }, planeMeshProxy->rotation);
    planeMeshProxy->rigidBody = rigidBodyPlane;
    physicsScene->AddRigidBody(rigidBodyPlane); //add to the physics scene

    rigidBodyPlane->mass = MMath::FLOAT_MAX; //set the mass to 0, so it is static
    rigidBodyPlane->simulatePhysics = false; //make the plane static, not affected by physics

    auto planeCollider = new Collider(planeMeshProxy, Plane{ planeMeshProxy->scale.x(), planeMeshProxy->scale.y() }, rigidBodyPlane); //plane size
    planeMeshProxy->collider = planeCollider;
    physicsScene->AddCollider(planeCollider); //add to the physics scene

    rigidBodyPlane->simulateRotation = false; //make the plane not affected by rotation, so it will not fall down due to gravity
    rigidBodyPlane->debugName = "Plane"; //set the debug name for the plane


    //m_staticMeshes.push_back(cubeMeshProxy0);
    m_staticMeshes.push_back(cubeMeshProxy1);
    m_staticMeshes.push_back(cubeMeshProxy2);
    //m_staticMeshes.push_back(cubeMeshProxy3);
    //m_staticMeshes.push_back(cubeMeshProxy4);
    //m_staticMeshes.push_back(cubeMeshProxy5);

    m_staticMeshes.push_back(sphereMeshProxy0);
    //m_staticMeshes.push_back(sphereMeshProxy1);
    //m_staticMeshes.push_back(sphereMeshProxy2);

    m_staticMeshes.push_back(planeMeshProxy);
}



std::vector<InstanceData> D3D12HelloRenderer::GenerateInstanceData()
{
    std::vector<InstanceData> instanceData;

    // Estimate cube grid dimensions (cubical or close)
    int gridSize = static_cast<int>(std::ceil(std::cbrt(instanceCount)));
    const float halfGrid = (gridSize - 1) * spacing * 0.5f;

    for (int i = 0; i < instanceCount; ++i)
    {
        int x = i % gridSize;
        int y = (i / gridSize) % gridSize;
        int z = i / (gridSize * gridSize);

        InstanceData _data;

        _data.offset = {
            x * spacing - halfGrid,
            y * spacing - halfGrid,
            z * spacing - halfGrid
        };

        instanceData.push_back(_data);
    }

    return instanceData;
}



// Update frame-based values.
void D3D12HelloRenderer::OnUpdate(float delta)
{
    for (auto& proxy : m_staticMeshes)
    {
        auto constBufferHandle = proxy->constantBuffer;
        if (constBufferHandle == nullptr) {
            continue;
        }

        //eye rotate around the origin
        constexpr float speedDivisor = 50.0f; // Increase this number to slow it down
        float angle = static_cast<float>((GetTickCount64() / static_cast<ULONGLONG>(speedDivisor)) % 360) * XM_PI / 180.0f;


        //auto yAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        //auto rotation = XMQuaternionRotationAxis(yAxis, delta);
        //proxy->rotation = XMQuaternionMultiply(rotation, proxy->rotation);

        SceneConstantBuffer constBufferData = {}; 

        auto modelMatrix_ = MMath::MatrixIdentity<float,4>();
        modelMatrix_ = MMath::MatrixScaling(proxy->scale.x(), proxy->scale.y(), proxy->scale.z()) * modelMatrix_;   

        auto R_ = XMMatrixRotationQuaternion(proxy->rotation);
		auto R = MMath::MatrixIdentity<float, 4>();
		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] , 0.0f };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] , 0.0f };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] , 0.0f };
		R[3] = { 0.0f, 0.0f, 0.0f, 1.0f };  

		R = MMath::Transpose(R); 
		modelMatrix_ = R * modelMatrix_; //rotate the model using the quaternion 

        //translate:
        auto translation = MMath::MatrixTranslation(proxy->position.x(), proxy->position.y(), proxy->position.z());
        modelMatrix_ = translation * modelMatrix_;


        //auto modelMatrix = XMMatrixIdentity();
        //modelMatrix = XMMatrixTranslation(proxy->position.x(), proxy->position.y(), proxy->position.z()) * modelMatrix;
        //modelMatrix = XMMatrixRotationQuaternion(proxy->rotation) * modelMatrix;
        //modelMatrix = XMMatrixScaling(proxy->scale.x(), proxy->scale.y(), proxy->scale.z()) * modelMatrix;

        // Translate the model to its position
        // Rotate the model using the quaternion


        //XMStoreFloat4x4(&constBufferData.modelMatrix, modelMatrix);
        constBufferData.modelMatrix = modelMatrix_; 


        //float eyePosX = cos(angle) * viewRadius;
        //float eyePosY = viewRadius * 0.3;
        //float eyePosZ = sin(angle) * viewRadius;

        float eyePosX = 0;
        float eyePosY = viewRadius * 0.2;
        float eyePosZ = -viewRadius;

        //Create view and projection matrices
       //LH = left-handed coordinate system
        //XMMATRIX view = XMMatrixLookAtLH(
        //    XMVectorSet(eyePosX, eyePosY, eyePosZ, 1.0f),  // Eye
        //    XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // At
        //    XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // Up
        //);

        auto view_ = MMath::LookAtLH(
        	{ eyePosX, eyePosY, eyePosZ }, // Eye
        	{ 0.0f, 0.0f, 0.0f },          // At
        	{ 0.0f, 1.0f, 0.0f }           // Up
        ); 

        //XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_aspectRatio, 0.1f, 100.0f);
        auto proj_ = MMath::PerspectiveFovLH(
        	MMath::ToRadians(45.0f),  
        	m_aspectRatio,  
        	0.1f,  
        	1000.0f  
        );

        //XMMATRIX vp = XMMatrixMultiply(view, proj);

        auto pv = proj_ * view_;
        //auto vp = MatrixMultiply(view, proj);

        //XMStoreFloat4x4(&constBufferData.viewProjectionMatrix, vp);
        constBufferData.projectionViewMatrix = pv;

         // Upload the constant buffer data.
        constBufferHandle->UploadData(&constBufferData, sizeof(SceneConstantBuffer));
    }


}