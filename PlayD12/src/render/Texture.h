#pragma once

#include "PCH.h"
#include "D12Helper.h"

#include "Samplers.h"


enum class ETextureType
{
    Texture2D,
    TextureCube,
    Texture2DArray,
};


enum class ETextureUsage : uint32_t {
    Undefined = 0,
    CopySrc = 1 << 0,
    CopyDest = 1 << 1,
    RenderTarget = 1 << 2,
    DepthStencil = 1 << 3,
    ShaderResource = 1 << 4,
    UnorderedAccess = 1 << 5,
    Present = 1 << 6,
    ResolveSrc = 1 << 7,
    ResolveDest = 1 << 8,
    DepthRead = 1 << 9,
};

inline ETextureUsage operator|(ETextureUsage a, ETextureUsage b) {
    return static_cast<ETextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool operator&(ETextureUsage a, ETextureUsage b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}
inline D3D12_RESOURCE_FLAGS GetResourceFlags(ETextureUsage usage) {
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    if (usage & ETextureUsage::RenderTarget) flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (usage & ETextureUsage::DepthStencil) flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (usage & ETextureUsage::UnorderedAccess) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    // SRV / Present doent require special flags, as they are just views on the resource.
    return flags;
}

inline D3D12_RESOURCE_STATES GetInitialResourceState(ETextureUsage usage) {
    if (usage & ETextureUsage::Present)        return D3D12_RESOURCE_STATE_PRESENT;
    if (usage & ETextureUsage::CopyDest)       return D3D12_RESOURCE_STATE_COPY_DEST;
    if (usage & ETextureUsage::CopySrc)        return D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (usage & ETextureUsage::ResolveSrc)     return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    if (usage & ETextureUsage::ResolveDest)    return D3D12_RESOURCE_STATE_RESOLVE_DEST;
    if (usage & ETextureUsage::UnorderedAccess)return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (usage & ETextureUsage::RenderTarget)   return D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (usage & ETextureUsage::DepthStencil)   return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (usage & ETextureUsage::DepthRead)      return D3D12_RESOURCE_STATE_DEPTH_READ;
    if (usage & ETextureUsage::ShaderResource) return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    return D3D12_RESOURCE_STATE_COMMON;
}

struct FTextureDesc {
    UINT width, height;
    DXGI_FORMAT format;
    std::optional<DXGI_FORMAT> dsvFormat{};
    std::optional<DXGI_FORMAT> srvFormat{};

    ETextureUsage usages; 
    // mip, array size, etc. 
    UINT mipLevels = 1;

    ETextureType type = ETextureType::Texture2D;
    UINT arraySize = 1;
};

class FD3D12Texture {
public:
    FD3D12Texture(ID3D12Device* device, const FTextureDesc& desc);

    //new: for swapchain backbuffer
   // FD3D12Texture(ID3D12Device* device, ComPtr<ID3D12Resource>, const FTextureDesc& desc);

    void UploadFromCPU(ID3D12GraphicsCommandList* cmd, 
        const void* data, 
        size_t rowPitch, 
        size_t slicePitch,
        UINT sliceIndex = 0,
        UINT sliceNum = 1
    );

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

namespace TexUtils {

    inline FTextureDesc CreateCubeTextureDesc() {
        FTextureDesc desc = {};
        desc.type = ETextureType::TextureCube;
        desc.arraySize = 6;  
        return desc;
    }

}