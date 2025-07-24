#pragma once

#include "PCH.h"

#include "D12Helper.h"

class FDescriptorHeapAllocator {

public:
	FDescriptorHeapAllocator(ComPtr<ID3D12Device> device,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		D3D12_DESCRIPTOR_HEAP_FLAGS Flags,
		UINT numDescriptors)
		: m_device(device), m_type(type), m_numDescriptors(numDescriptors)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = Flags; //D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; or D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
	}


	[[nodiscard]] UINT Allocate(UINT numDescriptors) {
		assert(m_currentIndex + numDescriptors <= m_numDescriptors );

		UINT startIndex = m_currentIndex;
		m_currentIndex += numDescriptors;
		return startIndex;
	}


	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT slot) {

		auto handle = m_heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += slot * m_device->GetDescriptorHandleIncrementSize(m_type);
		return handle;
	}

	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT slot) {
		auto handle = m_heap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += slot * m_device->GetDescriptorHandleIncrementSize(m_type);
		return handle;
	}

	ComPtr<ID3D12DescriptorHeap> GetHeap() const {
		return m_heap;
	}

	UINT GetCurrentOffset() const {
		return m_currentIndex;
	}

	void Reset() {
		m_currentIndex = 0;
	}

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	UINT m_numDescriptors;
	UINT m_currentIndex = 0;
};

