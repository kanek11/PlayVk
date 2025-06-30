#include "PCH.h"
#include "Resource.h"



FD3D12Buffer::FD3D12Buffer(ID3D12Device* m_device, const FBufferDesc& desc):
	m_device(m_device), m_bufferDesc(desc) 
{  
	CreateResource();
}

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

void FD3D12Buffer::CreateResource()
{
	auto heapType = GetHeapType(m_bufferDesc.Usage);
	auto initialState = GetInitialState(m_bufferDesc.Usage);

	auto bufferSize = static_cast<UINT64>(m_bufferDesc.SizeInBytes);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heapType),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		initialState,
		nullptr,
		IID_PPV_ARGS(&m_resource)));

}

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
	if (size > m_bufferDesc.SizeInBytes) {
		throw std::runtime_error("Upload data size exceeds buffer size");
	}
	void* mappedDataBegin = this->Map();
	memcpy(mappedDataBegin, data, size);
	this->Unmap();
}

 
