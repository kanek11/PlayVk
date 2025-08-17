#include "PCH.h"
#include "Resource.h"

//custom heap is rarely used.
static inline D3D12_HEAP_TYPE GetHeapType(const EBufferUsage& usage) {
	if ((usage & EBufferUsage::Upload) != EBufferUsage{}) return D3D12_HEAP_TYPE_UPLOAD;
	if ((usage & EBufferUsage::Readback) != EBufferUsage{}) return D3D12_HEAP_TYPE_READBACK;
	return D3D12_HEAP_TYPE_DEFAULT;
}

//in case bitmask matters for priorities, we may care about the order,
static inline D3D12_RESOURCE_STATES GetInitialState(const EBufferUsage& usage) {
	// usually most short-lived buffers are upload/readback 
	if ((usage & EBufferUsage::Upload) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	if ((usage & EBufferUsage::Readback) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_COPY_DEST;
	if ((usage & EBufferUsage::CopySrc) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	if ((usage & EBufferUsage::CopyDst) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_COPY_DEST;

	//  
	if ((usage & EBufferUsage::UAV) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	if ((usage & EBufferUsage::SRV) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	//less common but could be ;
	if ((usage & EBufferUsage::Index) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	if ((usage & EBufferUsage::Constant) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	if ((usage & EBufferUsage::Vertex) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	if ((usage & EBufferUsage::Indirect) != EBufferUsage{})
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

	//Fallback
	return D3D12_RESOURCE_STATE_COMMON;
}

// RT/DS is rarely used for buffers 
static inline D3D12_RESOURCE_FLAGS GetResourceFlags(const EBufferUsage& usage) {
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	if ((usage & EBufferUsage::UAV) != EBufferUsage{}) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	return flags;
}



//FD3D12Buffer::FD3D12Buffer(ID3D12Device* m_device, const FBufferDesc& desc) :
//	m_device(m_device), m_bufferDesc(desc)
//{
//	CreateResource();
//}

FD3D12Buffer::FD3D12Buffer(ID3D12Device* device, const FBufferDesc& desc) :
	m_device(device), m_bufferDesc(desc)
{
	CreateResource();
}

//void FD3D12Buffer::CreateResource()
//{
//	auto heapType = GetHeapType(m_bufferDesc.Usage);
//	auto initialState = GetInitialState(m_bufferDesc.Usage);
//
//	auto bufferSize = static_cast<UINT64>(m_bufferDesc.SizeInBytes);
//
//	ThrowIfFailed(m_device->CreateCommittedResource(
//		&CD3DX12_HEAP_PROPERTIES(heapType),
//		D3D12_HEAP_FLAG_NONE,
//		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, GetResourceFlags(m_bufferDesc.Usage)),
//		initialState,
//		nullptr,
//		IID_PPV_ARGS(m_resource.GetAddressOf())));
//
//}



void FD3D12Buffer::CreateResource()
{
	auto heapType = GetHeapType(m_bufferDesc.Usage);
	auto initialState = GetInitialState(m_bufferDesc.Usage);

	auto bufferSize = static_cast<UINT64>(m_bufferDesc.SizeInBytes);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heapType),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, GetResourceFlags(m_bufferDesc.Usage)),
		initialState,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf())));
}

/*
D3D12_VERTEX_BUFFER_VIEW FD3D12Buffer::GetVertexBufferView() const
{
	return D3D12_VERTEX_BUFFER_VIEW{
		m_resource->GetGPUVirtualAddress(),
		static_cast<UINT>(m_bufferDesc.SizeInBytes),
		static_cast<UINT>(m_bufferDesc.StrideInBytes)
	};
}

D3D12_INDEX_BUFFER_VIEW FD3D12Buffer::GetIndexBufferView() const
{
	return D3D12_INDEX_BUFFER_VIEW{
		m_resource->GetGPUVirtualAddress(),
		static_cast<UINT>(m_bufferDesc.SizeInBytes),
		m_bufferDesc.Format
	};
}

D3D12_CONSTANT_BUFFER_VIEW_DESC FD3D12Buffer::GetCBVDesc() const
{
	return D3D12_CONSTANT_BUFFER_VIEW_DESC{
		m_resource->GetGPUVirtualAddress(),
		static_cast<UINT>(m_bufferDesc.SizeInBytes)
	};
}

D3D12_SHADER_RESOURCE_VIEW_DESC FD3D12Buffer::GetSRVDesc() const
{
	UINT numElements = static_cast<UINT>(m_bufferDesc.SizeInBytes / m_bufferDesc.StrideInBytes);
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
		.Format = m_bufferDesc.Format,  //ok to remain DXGI_FORMAT_UNKNOWN
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Buffer = {
			.FirstElement = 0,
			.NumElements = numElements,
			.StructureByteStride = static_cast<UINT>(m_bufferDesc.StrideInBytes),
			.Flags = D3D12_BUFFER_SRV_FLAG_NONE
		}
	};
	return desc;

}

D3D12_UNORDERED_ACCESS_VIEW_DESC FD3D12Buffer::GetUAVDesc() const
{
	UINT numElements = static_cast<UINT>(m_bufferDesc.SizeInBytes / m_bufferDesc.StrideInBytes);
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {
		.Format = m_bufferDesc.Format,  //ok to remain DXGI_FORMAT_UNKNOWN
		.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
		.Buffer = {
			.FirstElement = 0,
			.NumElements = numElements,
			.StructureByteStride = static_cast<UINT>(m_bufferDesc.StrideInBytes),
			.CounterOffsetInBytes = 0,
			.Flags = D3D12_BUFFER_UAV_FLAG_NONE
		}
	};

	return desc;
}
*/





void* FD3D12Buffer::Map()
{
	void* mappedDataBegin = nullptr;

	CD3DX12_RANGE D3D12_GPU_VIRTUAL_ADDRESS_RANGE(0, 0);
	ThrowIfFailed(m_resource->Map(0, &D3D12_GPU_VIRTUAL_ADDRESS_RANGE, &mappedDataBegin));

	return mappedDataBegin;
}

void FD3D12Buffer::Unmap()
{
	m_resource->Unmap(0, nullptr);
}

void FD3D12Buffer::UploadData(const void* data, size_t size)
{
	assert(size <= m_bufferDesc.SizeInBytes);
	void* mappedDataBegin = this->Map();
	memcpy(mappedDataBegin, data, size);
	this->Unmap();
}

