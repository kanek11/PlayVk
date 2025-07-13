#pragma once  
#include "PCH.h"  

#include "Window.h" 
#include "D12Helper.h"

#include "Resource.h"
#include "Mesh.h"
#include "Shader.h"

#include "Physics/PhysicsScene.h"

#include "Math/MMath.h"

#include "Delegate.h"

#include "RenderPass.h"
 

struct MVPConstantBuffer
{
    //XMFLOAT4X4 modelMatrix; // 64 bytes  
    //XMFLOAT4X4 viewProjectionMatrix; // 64 bytes 
    FLOAT4X4 modelMatrix; // 64 bytes 
    FLOAT4X4 projectionViewMatrix; // 64 bytes

    //padding:
    float padding[32];
};

struct InstanceData
{
    MMath::FLOAT3 offset;
};

struct FInstanceProxy {
    std::vector<InstanceData> instanceData;
    SharedPtr<FD3D12Buffer> instanceBuffer;
};

struct FMaterialProxy {

	SharedPtr<FD3D12Texture> baseMap; 
};

//strip out the minimum to render a static mesh:
struct StaticMeshObjectProxy {
    FLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionIdentity();
    FLOAT3 scale = { 1.0f, 1.0f, 1.0f };

    SharedPtr<UStaticMesh> mesh;

    //material. 
    uint32_t mainPassHeapOffset = 0;
    SharedPtr<FMaterialProxy> material; 
    SharedPtr<FD3D12Buffer> mainMVPConstantBuffer;

	uint32_t shadowPassHeapOffset = 0; // for shadow pass
	SharedPtr<FD3D12Buffer> shadowMVPConstantBuffer; // for shadow pass


    SharedPtr<FInstanceProxy> instanceProxy;

    //new: for physics:
    RigidBody* rigidBody{ nullptr };
    Collider* collider{ nullptr };

    void SetWorldPosition(const FLOAT3& newPosition) {
        position = newPosition;
    }

    void SetWorldRotation(const DirectX::XMVECTOR& newRotation) {
        rotation = newRotation;
    }


    FDelegate<void(float)> onUpdate;
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
	void RecordRenderPassCommands(ID3D12GraphicsCommandList* commandList);

   
    ComPtr<ID3D12PipelineState> m_PSO;
    SharedPtr<FD3D12ShaderPermutation> m_shaderPerm;   

    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	SharedPtr<FD3D12Texture> m_depthStencil; 
	FRenderPassBindings m_bindings;  

    //---------

    void InitShadowPass();
	void BeginShadowPass(ID3D12GraphicsCommandList* commandList);
	void EndShadowPass(ID3D12GraphicsCommandList* commandList);
	void RecordShadowPassCommands(ID3D12GraphicsCommandList* commandList);


	ComPtr<ID3D12PipelineState> m_shadowPSO;
	SharedPtr<FD3D12ShaderPermutation> m_shadowShaderPerm;
      
    //shadowmap tex:
	SharedPtr<FD3D12Texture> m_shadowMap;
	FRenderPassBindings m_shadowBindings;

	UINT m_shadowMapWidth = 1024;  
	UINT m_shadowMapHeight = 1024;  
     

    RenderPassDesc shadowPassDesc = {
       .passTag = "Shadow",
       .colorFormat = DXGI_FORMAT_UNKNOWN, // no color
       .depthFormat = DXGI_FORMAT_D32_FLOAT,
       .enableDepth = true,
       .enableBlend = false,
       .cullMode = D3D12_CULL_MODE_BACK,
    };

     
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



    static_assert((sizeof(MVPConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
     
    //new: 6.27 texturing
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;  //bytes/channel (RGBA)
 
	SharedPtr<FD3D12Texture> m_fallBackTexture;  
    std::vector<UINT8> GenerateFallBackTextureData();


    std::vector<StaticMeshObjectProxy*> m_staticMeshes; 

    //todo:

    std::vector<InstanceData> GenerateInstanceData();

public:
    StaticMeshObjectProxy* InitMesh(SharedPtr<UStaticMesh> mesh, FLOAT3 position = { 0.0f, 0.0f, 0.0f }, FLOAT3 scale = { 1.0f, 1.0f, 1.0f });

    //todo:
    void SubmitMesh(StaticMeshObjectProxy* mesh);
    void ClearMesh();
    void SetMeshDescriptors();
    bool meshDirty = false;
};
