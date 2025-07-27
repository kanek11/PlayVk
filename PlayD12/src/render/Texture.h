#pragma once

#include "PCH.h"
#include "D12Helper.h"

#include "Samplers.h"



enum class ETextureUsage {
	Undefined = 0, 
    CopyDest,
    RenderTarget,
    DepthStencil,
    ShaderResource,
    UnorderedAccess,
	Present, 
    // ...
};

inline D3D12_RESOURCE_FLAGS GetResourceFlags(const std::vector<ETextureUsage>& usages) {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    for (auto usage : usages) {
        switch (usage) {
        case ETextureUsage::RenderTarget: flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; break;
        case ETextureUsage::DepthStencil: flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; break;
        case ETextureUsage::UnorderedAccess: flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; break; 
        default: break; // SRV ooesn't require special flags
        }
    }
    return flags;
}

inline D3D12_RESOURCE_STATES GetInitialResourceState(const std::vector<ETextureUsage>& usages) {
    // usually,sampling : PIXEL_SHADER_RESOURCE,
    // write: RTV or UAV
    for (auto& usage : usages) {
        switch (usage) {
        //case ETextureUsage::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
        case ETextureUsage::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ETextureUsage::DepthStencil: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case ETextureUsage::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case ETextureUsage::ShaderResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		case ETextureUsage::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;  
		case ETextureUsage::Present: return D3D12_RESOURCE_STATE_PRESENT; 
        }
    }
    return D3D12_RESOURCE_STATE_COMMON; // fallback
}

inline D3D12_RESOURCE_STATES GetInitialResourceState(const ETextureUsage& usage) { 
    switch (usage) { 
    //case ETextureUsage::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
    case ETextureUsage::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case ETextureUsage::DepthStencil: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case ETextureUsage::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case ETextureUsage::ShaderResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case ETextureUsage::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
    case ETextureUsage::Present: return D3D12_RESOURCE_STATE_PRESENT;
    } 
    return D3D12_RESOURCE_STATE_COMMON; // fallback
}


struct FTextureDesc {
    UINT width, height;
    DXGI_FORMAT format;
    std::optional<DXGI_FORMAT> dsvFormat{};
    std::optional<DXGI_FORMAT> srvFormat{};
    std::vector<ETextureUsage> usages; 
    //ETextureUsage initUsage = ETextureUsage::ShaderResource; 
    // mip, array size, etc.
};

class FD3D12Texture {
public:
    FD3D12Texture(ID3D12Device* device, const FTextureDesc& desc);

    //new: for swapchain backbuffer
	FD3D12Texture(ID3D12Device* device, ComPtr<ID3D12Resource>, const FTextureDesc& desc);

    void UploadFromCPU(ID3D12GraphicsCommandList* cmd, const void* data, size_t rowPitch, size_t slicePitch);

    ID3D12Resource* GetRawResource() const
    {
        return m_resource.Get();
    }

    FTextureDesc GetDesc() const
    {
        return m_desc;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
    D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDesc() const;
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc() const;

private:
    ComPtr<ID3D12Resource> m_resource;
    FTextureDesc m_desc;
	D3D12_RESOURCE_STATES m_initialState = D3D12_RESOURCE_STATE_COMMON;  

    //device:
    ID3D12Device* m_device = nullptr;

    //for a valid scope
    ComPtr<ID3D12Resource> uploadBuffer;
};

//SharedPtr<FD3D12Texture> CreateTextureForSwapChain(ID3D12Device* device, const FTextureDesc& desc)
//{
//	return CreateShared<FD3D12Texture>(device, desc);
//}
