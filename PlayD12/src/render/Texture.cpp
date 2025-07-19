#include "PCH.h"
#include "Texture.h"


D3D12_RESOURCE_FLAGS GetResourceFlags(const std::vector<ETextureUsage>& usages) {
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

D3D12_RESOURCE_STATES GetInitialResourceState(const std::vector<ETextureUsage>& usages) {
    // usually,sampling : PIXEL_SHADER_RESOURCE,
    // write: RTV or UAV
    for (auto usage : usages) {
        switch (usage) {
        case ETextureUsage::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ETextureUsage::DepthStencil: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case ETextureUsage::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case ETextureUsage::ShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }
    }
    return D3D12_RESOURCE_STATE_COMMON; // fallback
}

D3D12_CLEAR_VALUE MakeClearValue(const FTextureDesc& desc) {
    D3D12_CLEAR_VALUE cv = {};
    cv.Format = desc.dsvFormat.has_value() ? desc.dsvFormat.value() : desc.format;
    if (std::find(desc.usages.begin(), desc.usages.end(), ETextureUsage::RenderTarget) != desc.usages.end()) {
        cv.Color[0] = 0.0f;
        cv.Color[1] = 0.0f;
        cv.Color[2] = 0.0f;
        cv.Color[3] = 1.0f;
    }
    else if (std::find(desc.usages.begin(), desc.usages.end(), ETextureUsage::DepthStencil) != desc.usages.end()) {
        cv.DepthStencil.Depth = 1.0f;
        cv.DepthStencil.Stencil = 0;
    }

    return cv;
}

bool NeedsClearValue(const std::vector<ETextureUsage>& usages) {
    for (auto u : usages) {
        if (u == ETextureUsage::RenderTarget || u == ETextureUsage::DepthStencil)
            return true;
    }
    return false;
}



FD3D12Texture::FD3D12Texture(ID3D12Device* device, const FTextureDesc& desc)
    : m_device(device), m_desc(desc)
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = desc.width;
    texDesc.Height = desc.height;
    texDesc.DepthOrArraySize = 1;
    texDesc.Format = desc.format;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = GetResourceFlags(desc.usages);

    CD3DX12_RESOURCE_DESC resourceDesc(texDesc);

    //init state: 
    D3D12_RESOURCE_STATES initialState = GetInitialResourceState(desc.usages);

    //clear value for rtv/dsv:
    D3D12_CLEAR_VALUE clearValue;
    const D3D12_CLEAR_VALUE* pClearValue = nullptr;

    if (NeedsClearValue(desc.usages)) {
        clearValue = MakeClearValue(desc);
        pClearValue = &clearValue;
    }

    //default heap properties:
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            pClearValue,
            IID_PPV_ARGS(m_resource.GetAddressOf()))
    );
}

D3D12_SHADER_RESOURCE_VIEW_DESC FD3D12Texture::GetSRVDesc() const
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_desc.srvFormat.has_value() ? m_desc.srvFormat.value() : m_desc.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1; // assuming no mipmaps for simplicity
    return srvDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC FD3D12Texture::GetDSVDesc() const
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = m_desc.dsvFormat.has_value() ? m_desc.dsvFormat.value() : DXGI_FORMAT_UNKNOWN; // use the specified format or default
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE; // no special flags
    return dsvDesc;
}



void FD3D12Texture::UploadFromCPU(ID3D12GraphicsCommandList* cmdList, const void* data, size_t rowPitch,
    size_t slicePitch) {
    D3D12_SUBRESOURCE_DATA subresource = {};
    subresource.pData = data;
    subresource.RowPitch = rowPitch;
    subresource.SlicePitch = slicePitch;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);

    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    ThrowIfFailed(
        m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf()))
    );

    UpdateSubresources(cmdList, m_resource.Get(), uploadBuffer.Get(), 0, 0, 1, &subresource);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    cmdList->ResourceBarrier(1, &barrier);
}