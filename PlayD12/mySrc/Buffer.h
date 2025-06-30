#pragma once

#include "PCH.h"

#include "RHIHelper.h"

enum class EBufferUsage : uint32_t {
    Vertex = 1 << 0,
    Index = 1 << 1,
    Constant = 1 << 2,
    Structured = 1 << 3,
    CopySrc = 1 << 4,
    CopyDst = 1 << 5,
    UAV = 1 << 6,
    Upload = 1 << 7,
    Readback = 1 << 8
}; 

//enable bit operations for EBufferUsage
inline EBufferUsage operator|(EBufferUsage a, EBufferUsage b) {
	return static_cast<EBufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline EBufferUsage operator&(EBufferUsage a, EBufferUsage b) {
	return static_cast<EBufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
} 
inline EBufferUsage operator~(EBufferUsage a) {
	return static_cast<EBufferUsage>(~static_cast<uint32_t>(a));
}


D3D12_HEAP_TYPE GetHeapType(const EBufferUsage& usage) {
    if ((usage & EBufferUsage::Upload) != EBufferUsage{}) return D3D12_HEAP_TYPE_UPLOAD;
    if ((usage & EBufferUsage::Readback) != EBufferUsage{}) return D3D12_HEAP_TYPE_READBACK;
    return D3D12_HEAP_TYPE_DEFAULT;
}

D3D12_RESOURCE_STATES GetInitialState(const EBufferUsage& usage) {
    if ((usage & EBufferUsage::Upload) != EBufferUsage{}) return D3D12_RESOURCE_STATE_GENERIC_READ;
    if ((usage & EBufferUsage::Readback) != EBufferUsage{}) return D3D12_RESOURCE_STATE_COPY_DEST;
    if ((usage & EBufferUsage::CopyDst) != EBufferUsage{}) return D3D12_RESOURCE_STATE_COPY_DEST;
    return D3D12_RESOURCE_STATE_COMMON;
}



struct FBufferDesc {
    size_t SizeInBytes;  

    //fFor structured/UAV
    DXGI_FORMAT Format;
	size_t StrideInBytes;  

    // for engine 
    EBufferUsage Usage; 
    std::string DebugName;
};

class FD3D12Buffer {
public:
    FD3D12Buffer(ID3D12Device* device, const FBufferDesc& desc);
    ~FD3D12Buffer();

    // only if it's upload/readback
    void* Map();   
    void  Unmap();

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
     
    ID3D12Resource* GetResource() const { return m_resource.Get(); } 
private:
    void CreateResource();

    FBufferDesc m_desc;
    ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_initialState;
    bool m_isCPUAccessible = false;
};


