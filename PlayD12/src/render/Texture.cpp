#include "PCH.h"
#include "Texture.h"




D3D12_CLEAR_VALUE MakeClearValue(const FTextureDesc& desc) {
    D3D12_CLEAR_VALUE cv = {};
    cv.Format = desc.dsvFormat.has_value() ? desc.dsvFormat.value() : desc.format;
    if (desc.usages & ETextureUsage::RenderTarget) {
        cv.Color[0] = 0.0f;
        cv.Color[1] = 0.0f;
        cv.Color[2] = 0.0f;
        cv.Color[3] = 1.0f;
    }
    else if (desc.usages & ETextureUsage::DepthStencil) {
        cv.DepthStencil.Depth = 1.0f;
        cv.DepthStencil.Stencil = 0;
    }

    return cv;
}

bool NeedsClearValue(ETextureUsage usage) {
    if (usage & ETextureUsage::RenderTarget || usage & ETextureUsage::DepthStencil) {
        return true;
    }

    return false;
}



FD3D12Texture::FD3D12Texture(ID3D12Device* device, const FTextureDesc& desc)
    : m_device(device), m_desc(desc)
{ 

    //cubemap is inferred from srv;  simply set the array size;
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = desc.width;
    texDesc.Height = desc.height;
    texDesc.DepthOrArraySize = desc.arraySize;
    texDesc.Format = desc.format;
    texDesc.MipLevels = desc.mipLevels;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = GetResourceFlags(desc.usages);

    CD3DX12_RESOURCE_DESC resourceDesc(texDesc);

    //init state: 
    D3D12_RESOURCE_STATES initialState = GetInitialResourceState(desc.usages);
    this->m_initialState = initialState;

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
 


//the branching is kinda unavoidable and required,  the srv desc itself has union;
D3D12_SHADER_RESOURCE_VIEW_DESC FD3D12Texture::GetSRVDesc() const {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_desc.srvFormat.value_or(m_desc.format);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (m_desc.type) {
    case ETextureType::Texture2D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = m_desc.mipLevels;
        break;

    case ETextureType::Texture2DArray:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
        srvDesc.Texture2DArray.MipLevels = m_desc.mipLevels;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        break;

    case ETextureType::TextureCube:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = m_desc.mipLevels;
        srvDesc.TextureCube.MostDetailedMip = 0;
        break;
    }

    return srvDesc;
}


D3D12_RENDER_TARGET_VIEW_DESC FD3D12Texture::GetRTVDesc() const {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = m_desc.format;

    if (m_desc.type == ETextureType::Texture2D) {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
    }
    else { //Both Texture2DArray or Cube
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
        rtvDesc.Texture2DArray.FirstArraySlice = 0;
    }

    return rtvDesc;
}


D3D12_DEPTH_STENCIL_VIEW_DESC FD3D12Texture::GetDSVDesc() const {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = m_desc.dsvFormat.value_or(DXGI_FORMAT_UNKNOWN);
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    if (m_desc.type == ETextureType::Texture2D) {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    }
    else {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
        dsvDesc.Texture2DArray.FirstArraySlice = 0;
    }

    return dsvDesc;
}



D3D12_UNORDERED_ACCESS_VIEW_DESC FD3D12Texture::GetUAVDesc() const {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = m_desc.format;

    if (m_desc.type == ETextureType::Texture2D) {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
    }
    else {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.ArraySize = m_desc.arraySize;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
    }

    return uavDesc;
}


//FD3D12Texture::FD3D12Texture(ID3D12Device* device, ComPtr<ID3D12Resource> resource, const FTextureDesc& desc) :
//    m_device(device), m_desc(desc),
//    m_resource(resource)
//{
//
//}

void FD3D12Texture::UploadFromCPU(ID3D12GraphicsCommandList* cmdList, 
    const void* data, 
    size_t rowPitch,
    size_t slicePitch, 
    UINT sliceIndex,
    UINT sliceNum
) {
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

    auto preBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_resource.Get(),
        m_initialState,
        D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &preBarrier);


    UpdateSubresources(cmdList, 
        m_resource.Get(), 
        uploadBuffer.Get(), 0, 
        sliceIndex, sliceNum, &subresource);

    auto postBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        m_initialState);

    cmdList->ResourceBarrier(1, &postBarrier);
}