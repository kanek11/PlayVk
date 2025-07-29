#pragma once 
#include "PCH.h" 

#include "Base.h"
#include "D12Helper.h"

#include "Heap.h" 

#include <dxcapi.h>  // DXC headers
#include <d3d12shader.h> // for ID3D12ShaderReflection 


enum class EShaderStage {
	VERTEX,
	PIXEL,
	COMPUTE,
};

enum class ResourceScope { Scene, Object, Material, Unknown };

static ResourceScope InferScope(const std::string& name, UINT bindPoint) {
	if (bindPoint == 0 && name.find("Scene") != std::string::npos) return ResourceScope::Scene;
	if (bindPoint == 0 && name.find("Object") != std::string::npos) return ResourceScope::Object;
	if (bindPoint == 0 && name.find("Material") != std::string::npos) return ResourceScope::Object;
	return ResourceScope::Object;
}


struct ResourceBinding {
	//ResourceScope scope;
	//D3D_SHADER_INPUT_TYPE type;
	UINT bindPoint;
	//UINT space; 
	UINT localOffset = 0; // local offset in the descriptor table
	bool visibleVS = false;
	bool visiblePS = false;
};

struct FTableLayout {
	uint32_t tableSize;
	std::unordered_map<std::string, ResourceBinding> CBVBindings;
	std::unordered_map<std::string, ResourceBinding> SRVBindings;
	std::unordered_map<std::string, ResourceBinding> UAVBindings;
};


//a scope-aware layout , used to create root signature;
struct FRootSignatureLayout {
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> sceneRanges;
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> objectRanges;

	std::unordered_map<ResourceScope, UINT> rootIndexMap; 

	FTableLayout tableLayout; 

	std::unordered_map<std::string, ResourceBinding> samplerBindings;
};

//unprocessed information of shader parameters,
struct FShaderParameterMap {
	using SRVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using CBVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using UAVMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using SamplerMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>; //dynamic sampler;


	CBVMap cbvMap;
	SRVMap srvMap;
	UAVMap uavMap;
	SamplerMap samplerMap;
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

	std::optional<uint32_t> GetSamplerBindPoint(const std::string& name) const {
		if (this->parameterMap.samplerMap.contains(name)) {
			return this->parameterMap.samplerMap.at(name).BindPoint;
		}
		return std::nullopt;
	}


public:
	void Reflect(IDxcUtils* utils);

	FShaderParameterMap parameterMap;
	std::unordered_map<std::string, D3D12_SIGNATURE_PARAMETER_DESC> inputParameterMap;

	D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

private:
	ComPtr<IDxcBlobEncoding> m_shaderBlob;
};



class FD3D12ShaderPermutation {
public:
	FD3D12ShaderPermutation(ComPtr<ID3D12Device> device,
		WeakPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator
	)
		:m_device(device), m_rangeHeapAllocator(m_rangeHeapAllocator)
	{
	}

	void LoadShaders(IDxcUtils* dxcUtils, const std::string& assetPathVS, const std::string& assetPathPS) {
		m_vertexShader = CreateShared<FD3D12ShaderModule>(dxcUtils, assetPathVS);
		m_pixelShader = CreateShared<FD3D12ShaderModule>(dxcUtils, assetPathPS);
	}

	void LoadShaderCompute(IDxcUtils* dxcUtils, const std::string& assetPathCS) {
		m_computeShader = CreateShared<FD3D12ShaderModule>(dxcUtils, assetPathCS);
	}


	void ReflectRootSignatureLayout(IDxcUtils* dxcUtils);

	void CreateRootSignature();
	//  
	void SetSRV(const std::string& name, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& textureDesc, uint32_t baseOffset);
	void SetCBV(const std::string& name, const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, uint32_t baseOffset);
	void SetUAV(const std::string& name, ID3D12Resource* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc, uint32_t baseOffset);

	void SetStaticSampler(const std::string& name, const D3D12_STATIC_SAMPLER_DESC& samplerDesc);

	void SetDescriptorHeap(ID3D12GraphicsCommandList* commandList) const;
	void SetDescriptorTables(ID3D12GraphicsCommandList* commandList, uint32_t baseOffset) const;
	void SetDescriptorTablesCompute(ID3D12GraphicsCommandList* commandList, uint32_t baseOffset) const;
	void SetSceneRootCBV(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* sceneCB) const {
		assert(sceneCB != nullptr && "Scene constant buffer must not be null");
		if (resourceLayout.rootIndexMap.contains(ResourceScope::Scene)) {
			auto sceneCBIndex = resourceLayout.rootIndexMap.at(ResourceScope::Scene);
			cmdList->SetGraphicsRootConstantBufferView(sceneCBIndex, sceneCB->GetGPUVirtualAddress());
		}
		else {
			std::cerr << "Scene-level resource not found." << std::endl;
		}
	}
	void SetSceneRootCBVCompute(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* sceneCB) const {
		assert(sceneCB != nullptr && "Scene constant buffer must not be null");
		if (resourceLayout.rootIndexMap.contains(ResourceScope::Scene)) {
			auto sceneCBIndex = resourceLayout.rootIndexMap.at(ResourceScope::Scene);
			cmdList->SetComputeRootConstantBufferView(sceneCBIndex, sceneCB->GetGPUVirtualAddress());
		}
		else {
			std::cerr << "Scene-level resource not found." << std::endl;
		}
	}


	void AutoSetSamplers();

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

	ComPtr<ID3DBlob> GetCSBlob() const {
		if (m_computeShader) {
			ComPtr<ID3DBlob> csBlob;
			ThrowIfFailed(m_computeShader->GetShaderBlob().As(&csBlob));
			return csBlob;
		}
		return nullptr; // No compute shader loaded
	}

	//get descriptor heap:
	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const {
		return m_rangeHeapAllocator.lock()->GetHeap();
	}

	size_t GetDescriptorTableSize() const {
		return this->resourceLayout.tableLayout.tableSize;
	}

	uint32_t RequestAllocationOnHeap(size_t poolSize = 1) {
		//return m_rangeHeapAllocator.lock()->Allocate(this->m_rootSignatureLayout.numDescriptors);
		std::cout << "Request allocation on heap, num: " << poolSize << " table size: " << this->resourceLayout.tableLayout.tableSize << std::endl;

		return m_rangeHeapAllocator.lock()->Allocate((UINT)poolSize * this->resourceLayout.tableLayout.tableSize);
	}

private:
	SharedPtr<FD3D12ShaderModule> m_vertexShader;
	SharedPtr<FD3D12ShaderModule> m_pixelShader;
	SharedPtr<FD3D12ShaderModule> m_computeShader;


	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;

	FRootSignatureLayout resourceLayout;

	//owner
private:
	WeakPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;
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
	SharedPtr<FD3D12ShaderPermutation> GetOrLoadCompute(const ShaderPermutationKey& key);
private:
	ComPtr<IDxcUtils> dxcUtils;
	ComPtr<IDxcCompiler3> dxcCompiler;

	ComPtr<ID3D12Device> m_device;
	SharedPtr<FDescriptorHeapAllocator> m_rangeHeapAllocator;

private:
	std::unordered_map<ShaderPermutationKey, SharedPtr<FD3D12ShaderPermutation>> cache;


};
