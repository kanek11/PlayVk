#pragma once 
#include "PCH.h" 

#include "Base.h"
#include "D12Helper.h"

#include <dxcapi.h>  // DXC headers
#include <d3d12shader.h> // for ID3D12ShaderReflection 


//struct FD3D12ShaderBindings { 
//	uint32_t NumSRVs;
//	uint32_t NumCBVs;
//	uint32_t NumUAVs; 
//};
 

//the information of shader parameters,
struct FShaderParameterMap {
	using SRVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using CBVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using UAVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using SamplerMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>; //dynamic sampler;


	SRVMap srvMap; // Shader Resource Views
	CBVMap cbvMap; // Constant Buffer Views
	UAVMap uavMap; // Unordered Access Views
	SamplerMap samplerMap; // Samplers
};


struct FD3D12BindingLayout {
	D3D12_DESCRIPTOR_RANGE_TYPE type;
	UINT numDescriptors;
	UINT baseRegister;
	UINT heapLocalOffset;
};


struct FShaderDescriptorLayout {
	uint32_t tableSize;
	//UINT rootParameterIndex;

	FD3D12BindingLayout CBVLayout;
	std::unordered_map<std::string, uint32_t> CBVMemberOffsets; // offsets for each member in the CBV

	FD3D12BindingLayout SRVLayout;
	std::unordered_map<std::string, uint32_t> SRVMemberOffsets;

	FD3D12BindingLayout UAVLayout;
	std::unordered_map<std::string, uint32_t> UAVMemberOffsets;

	//FD3D12BindingLayout samplerLayout;
	//std::unordered_map<std::string, uint32_t> samplerMemberOffsets; 
};


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
		if (m_currentIndex + numDescriptors > m_numDescriptors) {
			throw std::runtime_error("allocator: Descriptor heap out of space");
		}
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




class FD3D12ShaderModule {
public:
	FD3D12ShaderModule() = default;
	FD3D12ShaderModule(IDxcUtils* utils, std::string path) 
	{ 
		// Load the shader from file
		auto widePath = std::wstring(path.begin(), path.end());
		ThrowIfFailed(utils->LoadFile(widePath.c_str(), nullptr, &m_shaderBlob)); 
	}

	//getter:
	ComPtr<IDxcBlob> GetShaderBlob() { return m_shaderBlob; }

	uint32_t GetParameterCount() const {
		return static_cast<uint32_t>(this->parameterMap.srvMap.size() +
			this->parameterMap.cbvMap.size() +
			this->parameterMap.uavMap.size());
		//+ this->parameterMap.samplerMap.size());
	}


	std::optional<uint32_t> GetHeapOffsetCBV(const std::string& name) const {
		if (this->tableLayout.CBVMemberOffsets.contains(name)) {
			return this->tableLayout.CBVMemberOffsets.at(name);
		}
		return std::nullopt;
	}

	std::optional<uint32_t> GetHeapOffsetSRV(const std::string& name) const {
		if (this->tableLayout.SRVMemberOffsets.contains(name)) {
			return this->tableLayout.SRVMemberOffsets.at(name);
		}
		return std::nullopt;
	}

	std::optional<uint32_t> GetHeapOffsetUAV(const std::string& name) const {
		if (this->tableLayout.UAVMemberOffsets.contains(name)) {
			return this->tableLayout.UAVMemberOffsets.at(name);
		}
		return std::nullopt;
	}

	bool HasSampler(const std::string& name) const {
		return this->parameterMap.samplerMap.contains(name);
	}

public:
	void Reflect(IDxcUtils* utils);
	void CreateDescriptorTable(uint32_t tableStartOffset);

	std::unordered_map<std::string, D3D12_SIGNATURE_PARAMETER_DESC> inputParameterMap;
	FShaderParameterMap parameterMap;
	D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
public:
	//similar to VkDescriptorSetLayout, retrieved by reflection; 
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;  //for valid scope
	D3D12_ROOT_PARAMETER1 descriptorTable{};
	FShaderDescriptorLayout tableLayout{};

private:
	ComPtr<IDxcBlobEncoding> m_shaderBlob;
};


enum class EShaderStage {
	VERTEX,
	PIXEL,
	COMPUTE,
};


struct FRootSignatureLayout {
	uint32_t numTables = 0; // Number of descriptor tables
	std::unordered_map<EShaderStage, UINT> tableIndexMap;

	uint32_t numDescriptors;
	uint32_t vsTableHeapOffset;
	uint32_t psTableHeapOffset;
};


class FD3D12ShaderPermutation {
public:
	FD3D12ShaderPermutation(ComPtr<ID3D12Device> device,
		WeakPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator
	)
		:m_device(device) , m_rangeHeapAllocator(m_rangeHeapAllocator)
	{ 
	} 

	void LoadShaders(IDxcUtils* dxcUtils, const std::string& assetPathVS, const std::string& assetPathPS) {
		m_vertexShader = CreateShared<FD3D12ShaderModule>(dxcUtils, assetPathVS);
		m_pixelShader = CreateShared<FD3D12ShaderModule>(dxcUtils, assetPathPS);
	}

	void PrepareRootSignature(IDxcUtils* dxcUtils);
	void CreateRootSignature();

	//  
	void SetSRV(const std::string& name, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& textureDesc, uint32_t baseOffset);
	void SetCBV(const std::string& name, ID3D12Resource* resource, const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, uint32_t baseOffset);
	void SetUAV(const std::string& name, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc, uint32_t baseOffset);

	void SetStaticSampler(const std::string& name, const D3D12_STATIC_SAMPLER_DESC& samplerDesc);

	void SetDescriptorHeap(ID3D12GraphicsCommandList* commandList) const;
	void SetDescriptorTables(ID3D12GraphicsCommandList* commandList, uint32_t baseOffset) const;


	//getter:
	ComPtr<ID3D12RootSignature> GetRootSignature() const {
		return m_rootSignature;
	}

	ComPtr<ID3DBlob> GetVSBlob() const {
		ComPtr<ID3DBlob> vsBlob;
		ThrowIfFailed(m_vertexShader->GetShaderBlob().As(&vsBlob));
		return vsBlob;
	}
	ComPtr<ID3DBlob> GetPSBlob() const {
		ComPtr<ID3DBlob> psBlob;
		ThrowIfFailed(m_pixelShader->GetShaderBlob().As(&psBlob));
		return psBlob;
	}

	//get descriptor heap:
	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const {
		return m_rangeHeapAllocator.lock()->GetHeap();
	}

	uint32_t RequestAllocationOnHeap() {
		return m_rangeHeapAllocator.lock()->Allocate(this->m_rootSignatureLayout.numDescriptors);
	}

	//utilis
public: 
	WeakPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers; // static samplers for the root signature

	//resources
public:
	SharedPtr<FD3D12ShaderModule> m_vertexShader;
	SharedPtr<FD3D12ShaderModule> m_pixelShader;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	FRootSignatureLayout m_rootSignatureLayout;

	//owner
private:
	ComPtr<ID3D12Device> m_device; 
};




struct ShaderPermutationKey {
	std::string shaderTag;
	std::string passTag;
	//std::set<std::string> defines;

	bool operator==(const ShaderPermutationKey& other) const {
		return shaderTag == other.shaderTag && passTag == other.passTag; /*&& defines == other.defines;*/
	}
};

namespace std {
	template<>
	struct hash<ShaderPermutationKey> {
		size_t operator()(const ShaderPermutationKey& k) const {
			size_t h1 = hash<std::string>()(k.shaderTag);
			size_t h2 = hash<std::string>()(k.passTag);

			return (h1 ^ (h2 << 1));
			//size_t h3 = 0;
			//for (const auto& d : k.defines) {
			//    h3 ^= hash<std::string>()(d);
			//}
			//return ((h1 ^ (h2 << 1)) ^ (h3 << 2));
		}
	};
}
 

class ShaderLibrary {
public:
	ShaderLibrary(ComPtr<ID3D12Device> device, SharedPtr<FDescriptorHeapAllocator> rangeHeapAllocator)
		: m_device(device), m_rangeHeapAllocator(rangeHeapAllocator)
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));

	} 

	[[nodiscard]]
	SharedPtr<FD3D12ShaderPermutation> GetOrLoad(const ShaderPermutationKey& key);
	 
private:
	ComPtr<IDxcUtils> dxcUtils;
	ComPtr<IDxcCompiler3> dxcCompiler;

	ComPtr<ID3D12Device> m_device;
	SharedPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;

private:
	std::unordered_map<ShaderPermutationKey, SharedPtr<FD3D12ShaderPermutation>> cache;


};

