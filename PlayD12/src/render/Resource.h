#pragma once 
#include "PCH.h"

#include "D12Helper.h"

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
	//D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
	//D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc() const; 

private: 

    FBufferDesc m_bufferDesc;
    ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_initialState; 

private:
	ID3D12Device* m_device = nullptr; 
};

