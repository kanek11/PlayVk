#pragma once 
#include "PCH.h"

#include "D12Helper.h"

#include "Base.h"

/*
typed = known layout,  turns out has very limited usage;  eg: index buffer;
most time we use structured = known stride;

*buffer class is designed to be thin;  separate with view = its semantics/handle;
* 
* we find view is not "innate" for the buffer;
* 
*if downstream wants simple syntax, expose helpers instead;
* 
* views are designed to be lightweight,
* you can just create a view in-place if the structure(type) is known;
* 
* we separate engine view vs descriptor view; as necessary complexity  

*view use weak ptr because it might be dangling;
* 


*/

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


//struct FBufferDesc {
//    size_t SizeInBytes;
//
//    // for structured/UAV
//    DXGI_FORMAT Format;
//    size_t StrideInBytes;
//
//    // for engine 
//    EBufferUsage Usage;
//    std::string DebugName;
//};


 
struct FBufferDesc {
    size_t SizeInBytes{ 0 };
     
    EBufferUsage Usage; 
};



class FD3D12Buffer {
public:
    //FD3D12Buffer(ID3D12Device* device, const FBufferDesc& desc);
    FD3D12Buffer(ID3D12Device* device, const FBufferDesc& desc);
	~FD3D12Buffer() = default;  //com ptr will handle release
     
    // only if it's upload/readback
    void* Map();
    void  Unmap();
    void UploadData(const void* data, size_t size);

	template <typename T>
	void UploadDataStructured(const T* data, size_t count = 1) {
		size_t size = count * sizeof(T);
		UploadData(data, size); 
	}
    template <typename T>
	void UploadDataStructureAt(const T* data, size_t offset, size_t count =1) { 
         
        size_t size = count * sizeof(T);
		UploadData(data + offset * sizeof(T), size);
    }


    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
        return m_resource->GetGPUVirtualAddress();
    }

    ID3D12Resource* GetRawResource() const { return m_resource.Get(); }

	FBufferDesc GetDesc() const {
		return m_bufferDesc;
	}

public:
    //buffer views:
    //D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    //D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
    //D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDesc() const;
    //D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const;
    //D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc() const;

private: 
    void CreateResource(); 

private:  
    FBufferDesc m_bufferDesc;
    ComPtr<ID3D12Resource> m_resource;

private:
    ID3D12Device* m_device = nullptr;
};
 

//also serve as the handle
struct FBufferView {
    WeakPtr<FD3D12Buffer> buffer;
    size_t offset;
    size_t size;
    size_t stride; 
	std::optional<DXGI_FORMAT> Format;  

    size_t ElementCount() const { return size / stride; }
    size_t FirstElement() const { return offset / stride; } 
};


  

namespace Buffer {
     
    struct FStructuredDesc {
        size_t count{ 1 };
        EBufferUsage Usage;
    };

	template <typename T>
	SharedPtr<FD3D12Buffer> CreateStructured(ID3D12Device* device, FStructuredDesc desc) {
		FBufferDesc bufferDesc = {
			.SizeInBytes = desc.count * sizeof(T),
			.Usage = desc.Usage
		};  
		auto buffer = CreateShared<FD3D12Buffer>(device, bufferDesc);
        return buffer;
	}
  

    //struct FBufferViewDesc {
    //    size_t offset;
    //    size_t size;
    //    size_t stride;
    //    std::optional<DXGI_FORMAT> Format;
    //};
    // 
	//inline FBufferView CreateBufferView(WeakPtr<FD3D12Buffer> buffer, const FBufferViewDesc& desc) {
	//	FBufferView view = {
	//		.buffer = buffer,
	//		.offset = desc.offset,
	//		.size = desc.size,
	//		.stride = desc.stride,
	//		.Format = desc.Format
	//	};
	//	return view;
	//} 

    /*
    * todo: we can always design more helpers if needed;
    //but that case we might use some kinda allocator instead;
    //so here we just assume full buffer
    */
     
	//bug fix: DXGI_FORMAT is compatible with integer type; not strongly typed;  
    //if theres more args, we should group into a struct;

    template <typename T>
	inline FBufferView CreateBVStructured(WeakPtr<FD3D12Buffer> buffer,
		std::optional<DXGI_FORMAT> format = std::nullopt
    )
    {
        auto& desc = buffer.lock()->GetDesc();
        //derived:
		size_t size = desc.SizeInBytes;
		size_t stride = sizeof(T); 

        FBufferView view = {
            .buffer = buffer,
			.offset = 0,
            .size = size,
            .stride = stride,
            .Format = format
        };
        return view;
    }

     
    //assumed full
    inline FBufferView CreateBVWhole(WeakPtr<FD3D12Buffer> buffer,
		std::optional<DXGI_FORMAT> format = std::nullopt 
    ) {
        auto& desc = buffer.lock()->GetDesc();

        FBufferView view = {
            .buffer = buffer,
			.offset = 0,
			.size = desc.SizeInBytes,
			.stride = desc.SizeInBytes, 
			.Format = format
        };
        return view;
    }



    inline D3D12_VERTEX_BUFFER_VIEW MakeVBV(const FBufferView& view) {
		assert(!view.buffer.expired());

        return D3D12_VERTEX_BUFFER_VIEW{
            .BufferLocation = view.buffer.lock()->GetGPUVirtualAddress() + (UINT64)view.offset,
            .SizeInBytes = static_cast<UINT>(view.size),
            .StrideInBytes = static_cast<UINT>(view.stride)
        };
    }

	inline D3D12_INDEX_BUFFER_VIEW MakeIBV(const FBufferView& view) {
		assert(!view.buffer.expired());
        assert(view.Format.has_value()); //format must be set for index buffer

		return D3D12_INDEX_BUFFER_VIEW{
			.BufferLocation = view.buffer.lock()->GetGPUVirtualAddress() + (UINT64)view.offset,
			.SizeInBytes = static_cast<UINT>(view.size),
			.Format = view.Format.value()
		};
	}


    inline D3D12_CONSTANT_BUFFER_VIEW_DESC MakeCBVDesc(const FBufferView& view) {
        assert(!view.buffer.expired());

		assert(view.size % 256 == 0); //constant buffer size must be aligned to 256 bytes; todo: a general "alignment policy";

        return D3D12_CONSTANT_BUFFER_VIEW_DESC{
            .BufferLocation = view.buffer.lock()->GetGPUVirtualAddress() + (UINT64)view.offset,
            .SizeInBytes = static_cast<UINT>(view.size),
        };
    }



	inline D3D12_SHADER_RESOURCE_VIEW_DESC MakeSRVDesc(const FBufferView& view) {
		assert(!view.buffer.expired());
		return D3D12_SHADER_RESOURCE_VIEW_DESC{
			.Format = view.Format.value_or(DXGI_FORMAT_UNKNOWN), //default to unknown
			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = static_cast<UINT>(view.FirstElement()),
				.NumElements = static_cast<UINT>(view.ElementCount()),
				.StructureByteStride = static_cast<UINT>(view.stride),
				.Flags = D3D12_BUFFER_SRV_FLAG_NONE
			}
		};
	}
	inline D3D12_UNORDERED_ACCESS_VIEW_DESC MakeUAVDesc(const FBufferView& view) {
		assert(!view.buffer.expired());
		return D3D12_UNORDERED_ACCESS_VIEW_DESC{
			.Format = view.Format.value_or(DXGI_FORMAT_UNKNOWN), //default to unknown
			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = static_cast<UINT>(view.FirstElement()),
				.NumElements = static_cast<UINT>(view.ElementCount()),
				.StructureByteStride = static_cast<UINT>(view.stride),
				.CounterOffsetInBytes = 0,
				.Flags = D3D12_BUFFER_UAV_FLAG_NONE
			}
		};
	}
}
 





template <typename T>
class FRingBufferAllocator {
public:
    constexpr static size_t stride = sizeof(T);

    FRingBufferAllocator(ID3D12Device* device, size_t poolSize)
        : m_poolSize(poolSize)
    { 
        FBufferDesc desc = {
            .SizeInBytes = stride * poolSize,
            .Usage = EBufferUsage::Upload | EBufferUsage::Constant 
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

    FBufferView Upload(const T* data, size_t size = 1) {
		//std::cout << "write index: " << m_writeIndex << ", size: " << m_poolSize << std::endl;
        assert(m_writeIndex < m_poolSize);

        uint8_t* dest = m_mappedPtr + m_writeIndex * stride;
        memcpy(dest, data, sizeof(T) * size);

        FBufferView view = {
            .buffer = m_buffer,
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