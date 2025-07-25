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


constexpr size_t MaxUIBatch = 256;
constexpr size_t MaxStaticMesh = 256;
constexpr size_t MaxLines = 1024;
constexpr uint32_t descriptorPoolSize = 4096;

struct RendererContext {
    ID3D12Device* device;

    ID3D12CommandAllocator* cmdAllocator;
    ID3D12GraphicsCommandList* cmdList;
    ID3D12CommandQueue* cmdQueue;

    SharedPtr<ShaderLibrary> shaderManager;
    SharedPtr<PSOManager> psoManager;
};

struct RenderGraphContext { 
    UINT shadowMapWidth;
    UINT shadowMapHeight;
    SharedPtr<FD3D12Texture> shadowMap;

    SharedPtr<FD3D12Texture> fallBackTexture; 
};


struct FrameContext {
     //std::vector<StaticMeshActorProxy*> staticMeshes;  
     std::vector<FStaticMeshProxy> staticMeshes;
     FCameraProxy* mainCamera;
};

namespace Render {
    inline RendererContext* rendererContext = nullptr;
    inline RenderGraphContext* graphContext = nullptr;
    inline FrameContext* frameContext = nullptr;
}
 


struct MVPConstantBuffer
{
    //XMFLOAT4X4 modelMatrix; // 64 bytes  
    //XMFLOAT4X4 viewProjectionMatrix; // 64 bytes 
    Float4x4 modelMatrix; // 64 bytes 
    Float4x4 projectionViewMatrix; // 64 bytes

    //padding:
    float padding[32];
};


static_assert((sizeof(MVPConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


//struct DebugLine {
//    Float3 start;
//    Float4 color0;
//    Float3 end;
//    Float4 color1;
//};

struct DebugLineVertex {
    Float3 position;
    Float4 color;
};

//
template<>
struct VertexLayoutTraits<DebugLineVertex> {
    static constexpr bool is_specialized = true;
    static constexpr auto attributes = std::to_array<VertexAttribute>({
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, offsetof(DebugLineVertex, position) },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, offsetof(DebugLineVertex, color) }
        });
};


class DebugDraw {
public:
    static DebugDraw& Get();

    void Init(const RendererContext* ctx);

    void OnUpdate(float delta, const Float4x4& pv);

    void AddRay(const Float3& origin, const Float3& direction,
        const Float4& color0 = Float4(1.0f, 1.0f, 1.0f, 1.0f),
        const Float4& color1 = Float4(1.0f, 1.0f, 1.0f, 1.0f)
    );
    void AddLine(const Float3& start, const Float3& end,
        const Float4& color = Float4(1.0f, 0.0f, 0.0f, 1.0f),
        const Float4& color1 = Float4(1.0f, 1.0f, 1.0f, 1.0f));

    void FlushAndRender(ID3D12GraphicsCommandList* cmdList);

    //static_assert(sizeof(DebugLineVertex) == sizeof(float) * 7, "unexpected vertex size?");
private:
    void Clear()
    {
        m_lineData.clear();
    }
private:
    std::vector<DebugLineVertex> m_lineData;
    SharedPtr<FD3D12Buffer> m_vertexBuffer;
    SharedPtr<FD3D12ShaderPermutation> m_shader;
    ComPtr<ID3D12PipelineState> m_PSO;

    //MVP buffer:
    SharedPtr<FD3D12Buffer> m_CB;
    uint32_t heapOffset;


    MaterialDesc m_materialDesc = {
        .shaderTag = "Debug",

        .enableAlphaBlend = true,
        .doubleSided = true,
        .depthWrite = false
    };

    RenderPassDesc m_renderPassDesc = {
        .passTag = "Line",
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        .depthFormat = DXGI_FORMAT_UNKNOWN,
        .enableDepth = false,
        .cullMode = D3D12_CULL_MODE_NONE,
    };

};
 


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

    void LoadPipeline();
    void LoadAssets();


    //void PopulateCommandList();
    void BeginFrame();
    void EndFrame();

    void WaitForPreviousFrame();


    //---------
    void InitRenderPass();
    void BeginRenderPass(ID3D12GraphicsCommandList* commandList);
    void EndRenderPass(ID3D12GraphicsCommandList* commandList);
    //void RecordRenderPassCommands(ID3D12GraphicsCommandList* commandList);


    //ComPtr<ID3D12PipelineState> m_PSO;
    //SharedPtr<FD3D12ShaderPermutation> m_shaderPerm;

    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    SharedPtr<FD3D12Texture> m_depthStencil;
    FRenderPassBindings m_bindings;
     
    //---------

    void InitShadowPass();
    void BeginShadowPass(ID3D12GraphicsCommandList* commandList);
    void EndShadowPass(ID3D12GraphicsCommandList* commandList);
    //void RecordShadowPassCommands(ID3D12GraphicsCommandList* commandList); 

    //ComPtr<ID3D12PipelineState> m_shadowPSO;
    //SharedPtr<FD3D12ShaderPermutation> m_shadowShaderPerm;

    //shadowmap tex:
    SharedPtr<FD3D12Texture> m_shadowMap;
    FRenderPassBindings m_shadowBindings;

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

    SharedPtr<FD3D12Texture> m_fallBackTexture;
    std::vector<UINT8> GenerateFallBackTextureData();

public: 

    //std::vector<StaticMeshActorProxy*> m_staticMeshes;  
    std::vector<FStaticMeshProxy> staticMeshes;
    FCameraProxy* mainCamera; 

    //todo:
    void SubmitMesh(StaticMeshActorProxy* mesh);
    void ClearMesh();
    //void SetMeshDescriptors();
    //bool meshDirty = false; 

    void SubmitCamera(FCameraProxy* camera);


public:
    Lit::PassContext litPassCtx;

    public:
        Shadow::PassContext shadowPassCtx;


public:
    //SharedPtr<DebugRenderer> m_debugRenderer;
    //SharedPtr<UIRenderer> uiRenderer;

    UI::UIPassContext uiPassCtx;

    void AddQuad(const UI::FQuadDesc& desc);

    void AddQuad(const FRect& rect, const Float4& color);
    void AddQuad(const FRect& rect, const Float2& uvTL, const Float2& uvBR);

    void ClearUI()
    {
        UI::EndFrame(uiPassCtx);
    }

    SharedPtr<FontAtlas> GetFontAtlas() const {
        return uiPassCtx.data.font;
    }

};