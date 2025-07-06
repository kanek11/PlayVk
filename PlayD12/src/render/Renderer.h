#pragma once  
#include "PCH.h"  

#include "Window.h" 
#include "D12Helper.h"
 
#include "Resource.h"
#include "Mesh.h"
#include "Shader.h"

#include "Physics/PhysicsScene.h"

#include "Math/MMath.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;


struct InstanceData
{
    MMath::FLOAT3 offset;
};

struct FInstanceProxy {
    std::vector<InstanceData> instanceData;
    SharedPtr<FD3D12Buffer> instanceBuffer;
};

struct FMaterialProxy {
    ComPtr<ID3D12Resource> baseMapResource;
    D3D12_SHADER_RESOURCE_VIEW_DESC baseMapSRV;
};

//strip out the minimum to render a static mesh:
struct StaticMeshObjectProxy {
    FLOAT3 position = { 0.0f, 0.0f, 0.0f };
    XMVECTOR rotation = XMQuaternionIdentity();
    FLOAT3 scale = { 1.0f, 1.0f, 1.0f };

    SharedPtr<UStaticMesh> mesh;
    uint32_t heapStartOffset = 0;

    //material.
    SharedPtr<FMaterialProxy> material;

    SharedPtr<FD3D12Buffer> constantBuffer;

    SharedPtr<FInstanceProxy> instanceProxy;

    //new: for physics:
    RigidBody* rigidBody{ nullptr };
    Collider* collider{ nullptr };

    void SetWorldPosition(const FLOAT3& newPosition) {
        position = newPosition;
    }

	void SetWorldRotation(const XMVECTOR& newRotation) {
		rotation = newRotation;
	}
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
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;

    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;


    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize;

    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;


    // App resources.


    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();


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

    //new: enable depth:
    //Create a Depth/Stencil Buffer Resource
    ComPtr<ID3D12Resource> m_depthStencil;
    //Create a Depth Stencil View (DSV) Descriptor Heap
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;


    SharedPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;

    //ComPtr<ID3D12RootSignature> m_rootSignature; 
    SharedPtr<FD3D12GraphicsShaderManager> m_shaderManager;


    struct SceneConstantBuffer
    {
        XMFLOAT4X4 modelMatrix; // 64 bytes  
        XMFLOAT4X4 viewProjectionMatrix; // 64 bytes 
		//FLOAT4X4 modelMatrix; // 64 bytes 
		//FLOAT4X4 projectionViewMatrix; // 64 bytes

        //padding:
        float padding[32];
    };

    static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");



    //new: 6.27 texturing
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;  //bytes/channel (RGBA)

    ComPtr<ID3D12Resource> m_fallBackTexture;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_fallBackSRVDesc;
    std::vector<UINT8> GenerateFallBackTextureData(); 


    std::vector<StaticMeshObjectProxy*> m_staticMeshes;


    void InitMeshAssets();
    void SetMeshDescriptors();

    StaticMeshObjectProxy* InitMesh(SharedPtr<UStaticMesh> mesh, FLOAT3 position = { 0.0f, 0.0f, 0.0f }, FLOAT3 scale = { 1.0f, 1.0f, 1.0f });

    std::vector<InstanceData> GenerateInstanceData();
};

