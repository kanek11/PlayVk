#pragma once 
#include "PCH.h"

#include "D12Helper.h"

#include "Base.h"

//alignment helper
inline size_t Align256(size_t s) { return (s + 255) & ~255ull; }




enum class EBufferUsage : uint32_t {
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Constant = 1 << 2,
    Upload = 1 << 3,
    Readback = 1 << 4,
    CopySrc = 1 << 5,
    CopyDst = 1 << 6,
    Structured = 1 << 7,
    UAV = 1 << 8,
	SRV = 1 << 9,
	Indirect = 1 << 10, // for indirect draw/dispatch 
};

//enable bit operations for EBufferUsage
inline EBufferUsage operator|(EBufferUsage a, EBufferUsage b) {
    return static_cast<EBufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline EBufferUsage operator&(EBufferUsage a, EBufferUsage b) {
    return static_cast<EBufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
//inline EBufferUsage operator~(EBufferUsage a) {
//    return static_cast<EBufferUsage>(~static_cast<uint32_t>(a));
//}


struct FBufferDesc {
    size_t SizeInBytes;

    // for structured/UAV
    DXGI_FORMAT Format;
    size_t StrideInBytes;

    // for engine 
    EBufferUsage Usage;
    std::string DebugName;
};

class FD3D12Buffer {
public:
    FD3D12Buffer(ID3D12Device* device, const FBufferDesc& desc);
    ~FD3D12Buffer() = default;  //todo: resource release


    void CreateResource();

    // only if it's upload/readback
    void* Map();
    void  Unmap();
    void UploadData(const void* data, size_t size);

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
        return m_resource->GetGPUVirtualAddress();
    }

    ID3D12Resource* GetRawResource() const { return m_resource.Get(); }

public:
    //buffer views:
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
    D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDesc() const;
    D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc() const; 

private:

    FBufferDesc m_bufferDesc;
    ComPtr<ID3D12Resource> m_resource;

private:
    ID3D12Device* m_device = nullptr;
};



struct FBufferView {
    FD3D12Buffer* buffer;
    size_t offset;
    size_t size;
    size_t stride;
};

inline D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDesc(const FBufferView& view) {
    return D3D12_CONSTANT_BUFFER_VIEW_DESC{
        .BufferLocation = view.buffer->GetGPUVirtualAddress() + (UINT64)view.offset,
        .SizeInBytes = static_cast<UINT>(view.size),
    };
}

inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(const FBufferView& view) {
    return D3D12_VERTEX_BUFFER_VIEW{
     .BufferLocation = view.buffer->GetGPUVirtualAddress() + (UINT64)view.offset,
    .SizeInBytes = static_cast<UINT>(view.size),
    .StrideInBytes = static_cast<UINT>(view.stride)
    };
}


template <typename T>
class FRingBufferAllocator {
public:
    constexpr static size_t stride = sizeof(T);

    FRingBufferAllocator(ID3D12Device* device, size_t poolSize)
        : m_poolSize(poolSize)
    {
        size_t totalSize = stride * poolSize;

        FBufferDesc desc = {
            .SizeInBytes = totalSize,
            .Format = DXGI_FORMAT_UNKNOWN,
            .StrideInBytes = stride,
            .Usage = EBufferUsage::Upload | EBufferUsage::Constant,
            .DebugName = "Unnamed RingBuffer"
        };
        m_buffer = CreateShared<FD3D12Buffer>(device, desc);
        m_mappedPtr = reinterpret_cast<uint8_t*>(m_buffer->Map());
    }

    ~FRingBufferAllocator() {
        if (m_buffer) m_buffer->Unmap();
    }

    void Reset() {
        m_writeIndex = 0;
    }

    FBufferView Upload(const T* data,  size_t size = 1) {
        assert(m_writeIndex < m_poolSize);

        uint8_t* dest = m_mappedPtr + m_writeIndex * stride;
        memcpy(dest, data, sizeof(T) * size); 

        FBufferView view = {
            .buffer = m_buffer.get(),
            .offset = m_writeIndex * stride,
            .size = sizeof(T) * size,
            .stride = stride,
        };

        m_writeIndex += size;
        return view;
    }


private:
    SharedPtr<FD3D12Buffer> m_buffer;
    uint8_t* m_mappedPtr = nullptr;

    size_t m_writeIndex = 0;
    size_t m_poolSize = 0;
};
