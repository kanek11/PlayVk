#pragma once  
#include "PCH.h"  

#include "Window.h" 
#include "D12Helper.h"

#include "Resource.h"
#include "Mesh.h"
#include "Shader.h"
#include "Text.h"

#include "Math/MMath.h"

#include "pipeline.h" 
#include "RenderPass.h"

#include "UI.h"

#include "StaticMeshActor.h"

#include "UIPass.h" 
#include "GeometryPass.h"
#include "DebugRay.h"
#include "PostPass.h"

#include "Probe.h"


constexpr size_t MaxUIBatch = 128;
constexpr size_t MaxStaticMesh = 128;
constexpr size_t MaxLines = 1024;
constexpr uint32_t descriptorPoolSize = 4096;

using UploadTask = std::function<void(ID3D12GraphicsCommandList* cmdList)>;


struct FResourcePool {


};


struct RendererContext {
    ID3D12Device* device;

    ID3D12CommandAllocator* cmdAllocator;
    ID3D12GraphicsCommandList* cmdList;
    ID3D12CommandQueue* cmdQueue;

    SharedPtr<ShaderLibrary> shaderManager;
    SharedPtr<PSOManager> psoManager;


    SharedPtr<FDescriptorHeapAllocator> dsvHeapAllocator;
    SharedPtr<FDescriptorHeapAllocator> rtvHeapAllocator;
};


struct FrameDataContext {
    //std::vector<StaticMeshActorProxy*> staticMeshes;  
    std::vector<FStaticMeshProxy> staticMeshes;
    FCameraProxy* mainCamera;

    FD3D12Buffer* sceneCB;
    //current present rtv:
    D3D12_CPU_DESCRIPTOR_HANDLE currentRTV;
};

struct RenderGraphContext {
    //RenderGraph* graph;

    UINT shadowMapWidth;
    UINT shadowMapHeight;

    SharedPtr<FD3D12Texture> shadowMap;

    std::unordered_map<std::string, SharedPtr<FD3D12Texture>> gbuffers;
    SharedPtr<FD3D12Texture> sceneDepth;

    SharedPtr<FD3D12Texture> skybox;

    std::unordered_map<std::string, SharedPtr<FD3D12Texture>> loadedTextures;
};


namespace Render {
    inline RendererContext* rendererContext = nullptr;
    inline FrameDataContext* frameContext = nullptr;

    inline RenderGraphContext* graphContext = nullptr;
}


class D3D12HelloRenderer
{
public:
    D3D12HelloRenderer(UINT width, UINT height, std::wstring name, SharedPtr<WindowBase>);

    virtual void OnInit();
    virtual void OnUpdate(float delta);
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;

    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;


    //ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    //UINT m_rtvDescriptorSize; 
    SharedPtr<FDescriptorHeapAllocator> m_SC_RTVHeapAllocator; //new: for range allocation

    // App resources.

    ///ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    SharedPtr<FDescriptorHeapAllocator> m_dsvHeapAllocator;
    SharedPtr<FDescriptorHeapAllocator> m_rtvHeapAllocator;

    //shader visible descriptor heap for CBV/SRV/UAV:
    SharedPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;



    // Synchronization objects.
    UINT m_frameIndex{ 0 };
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    //void PopulateCommandList();
    void BeginFrame();
    void EndFrame();

    void WaitForPreviousFrame();


    void LoadPipeline();
    void LoadPipelineCommon();
    void LoadSystemResources();



    D3D12_CPU_DESCRIPTOR_HANDLE AllocateCustomRT(FD3D12Texture* tex)
    {
        auto currIndex = m_rtvHeapAllocator->Allocate(1);
        auto rtvHandle = m_rtvHeapAllocator->GetCPUHandle(currIndex);
        m_device->CreateRenderTargetView(tex->GetRawResource(), nullptr, rtvHandle);
        return rtvHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE AllocateCustomDS(FD3D12Texture* tex)
    {
        auto currIndex = m_dsvHeapAllocator->Allocate(1);
        auto dsvHandle = m_dsvHeapAllocator->GetCPUHandle(currIndex);
        m_device->CreateDepthStencilView(tex->GetRawResource(), nullptr, dsvHandle);
        return dsvHandle;
    }


    //-------- 
    SharedPtr<FD3D12Buffer> sceneCB;


    //---------
    void InitPresentPass();
    void BeginPresentPass(ID3D12GraphicsCommandList* commandList);
    void EndPresentPass(ID3D12GraphicsCommandList* commandList);
    //void RecordRenderPassCommands(ID3D12GraphicsCommandList* commandList);

    D3D12_RENDER_TARGET_VIEW_DESC sc_RTV{};
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

    SharedPtr<FD3D12Texture> m_depthStencil; 
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsv;

    FRenderPassAttachments m_presentBindings;

    //---------

    void InitShadowPass();
    void BeginShadowPass(ID3D12GraphicsCommandList* commandList);
    void EndShadowPass(ID3D12GraphicsCommandList* commandList);
    //void RecordShadowPassCommands(ID3D12GraphicsCommandList* commandList); 

    //shadowmap tex:
    SharedPtr<FD3D12Texture> m_shadowMap;
    FRenderPassAttachments m_shadowBindings;

    UINT m_shadowMapWidth = 2048;
    UINT m_shadowMapHeight = 2048;


    //---------
    //new: 
    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    // Adapter info.
    bool m_useWarpDevice;

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    //new:
    SharedPtr<WindowBase> m_mainWindow;

    //ComPtr<ID3D12RootSignature> m_rootSignature; 

    SharedPtr<ShaderLibrary> m_shaderManager;

    SharedPtr<PSOManager> m_psoManager;

    //new: 6.27 texturing
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;  //bytes/channel (RGBA)

    std::vector<UINT8> GenerateCheckerboardData();
    std::vector<UINT8> checkerBoardData;
public:

    //std::vector<StaticMeshActorProxy*> m_staticMeshes;  
    std::vector<FStaticMeshProxy> staticMeshes;
    FCameraProxy* mainCamera;

    //todo:
    void SubmitMesh(StaticMeshActorProxy* mesh);
    void ClearMesh();

    void SubmitCamera(FCameraProxy* camera);


public:
    Lit::PassContext litPassCtx;

public:
    Shadow::PassContext shadowPassCtx;

public:
    //SharedPtr<DebugRenderer> m_debugRenderer;
    //SharedPtr<UIRenderer> uiRenderer;

    UI::UIPassContext uiPassCtx;

    void AddQuad(const FQuadDesc& desc);

    void AddQuad(const FRect& rect, const Float4& color);
    void AddQuad(const FRect& rect, const Float2& uvTL, const Float2& uvBR);

    void ClearUI()
    {
        UI::EndFrame(uiPassCtx);
    }

    SharedPtr<FontAtlas> GetFontAtlas() const {
        return uiPassCtx.font;
    }


public:
    GBuffer::PassContext gbufferPassCtx;
     
    FRenderPassAttachments m_gbufferAttachments;

    void InitGBuffers();
    void BeginGBufferPass(ID3D12GraphicsCommandList* commandList);
    void EndGBufferPass(ID3D12GraphicsCommandList* commandList);


public:
    PBR::PassContext pbrShadingCtx;


//public:
//    Compute::ComputeContext computeCtx;
// 

public:
    void InitEnvMap();

    FProbe probe; 

public:

    void UploadTexture(std::vector<UploadTask> tasks);

public:



    std::unordered_map<std::string, SharedPtr<FD3D12Texture>> loadTextures;
     
    RenderGraphContext* m_graph = new RenderGraphContext();
};