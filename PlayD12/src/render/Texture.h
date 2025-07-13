#pragma once

#include "PCH.h"
#include "D12Helper.h"



enum class ETextureUsage {
    RenderTarget,
    DepthStencil,
    ShaderResource,
    UnorderedAccess,
    // ...
};

struct FTextureDesc {
    UINT width, height;
    DXGI_FORMAT format;
    std::optional<DXGI_FORMAT> dsvFormat{};  
    std::optional<DXGI_FORMAT> srvFormat{};
    std::vector<ETextureUsage> usages;
    // mip, array size, etc.
};

class FD3D12Texture {
public:
    FD3D12Texture(ID3D12Device* device, const FTextureDesc& desc);

	void UploadFromCPU(ID3D12GraphicsCommandList* cmd, const void* data, size_t rowPitch);
     

    ID3D12Resource* GetRawResource() const
	{
		return m_resource.Get();
	}

    D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const; 
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDesc() const;
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc() const;

private:
    ComPtr<ID3D12Resource> m_resource;
    FTextureDesc m_desc;

    //device:
	ID3D12Device* m_device = nullptr;

    ComPtr<ID3D12Resource> uploadBuffer;
};
