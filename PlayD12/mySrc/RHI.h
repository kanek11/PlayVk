#pragma once
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
 
#include "PCH.h"  
#include "Window.h" 
#include "RHIHelper.h"

#include "Mesh.h"
#include "Shader.h"


using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12HelloTriangle2
{
public:
    D3D12HelloTriangle2(UINT width, UINT height, std::wstring name, SharedPtr<WindowBase>);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    //struct Vertex
    //{
    //    XMFLOAT3 position;
    //    XMFLOAT4 color;
    //};

    //new:
    struct SceneConstantBuffer
    {
        XMFLOAT4 offset;
    
        //the vpMatrix:
        XMFLOAT4X4 viewProjectionMatrix; // 64 bytes 
        //padding:
        float padding[108]; 
    };

    static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize;
	//ComPtr<ID3D12DescriptorHeap> m_cbvHeap; //new: 6.26 const buffer 
	//ComPtr<ID3D12DescriptorHeap> m_srvHeap; //new: 6.27 texturing
	//ComPtr<ID3D12DescriptorHeap> m_cbv_srv_heap;  
	//UINT m_cbvSrvDescriptorSize; 
    
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    

    // App resources.

	//new: 6.26
    ComPtr<ID3D12Resource> m_constantBuffer;
    SceneConstantBuffer m_constantBufferData;
    UINT8* m_pCbvDataBegin;

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
      
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    //new: add index buffer:
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    CubeMesh m_cubeMesh = CubeMesh(); 


    //new: instancing:
	struct InstanceData
	{
        XMFLOAT3 offset; 
	};

	std::vector<InstanceData> m_instanceData;
	ComPtr<ID3D12Resource> m_instanceBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_instanceBufferView;


	//new: enable depth:
    //Create a Depth/Stencil Buffer Resource
	ComPtr<ID3D12Resource> m_depthStencil;
    //Create a Depth Stencil View (DSV) Descriptor Heap
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
 


    //new: 6.27 texturing
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
	static const UINT TexturePixelSize = 4;  //bytes/channel (RGBA)

    ComPtr<ID3D12Resource> m_texture;
    std::vector<UINT8> GenerateTextureData();




    //ComPtr<ID3D12RootSignature> m_rootSignature;

    //new: dxc workflow
 //   ComPtr<IDxcUtils> dxcUtils;
 //   ComPtr<IDxcCompiler3> dxcCompiler;


	//SharedPtr<FD3D12ShaderModule> m_vertexShader;
	//SharedPtr<FD3D12ShaderModule> m_pixelShader;

	//SharedPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;

	SharedPtr<FD3D12GraphicsShaderManager> m_shaderManager;

};



//todo: : add texture:
// ComPtr<ID3D12Resource> m_texture;
// ComPtr<ID3D12DescriptorHeap> m_srvHeap;
// D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
