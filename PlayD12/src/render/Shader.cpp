#include "PCH.h"
#include "Shader.h"

#include "Application.h"
 

void FD3D12ShaderModule::Reflect(IDxcUtils* utils)
{
	assert(m_shaderBlob != nullptr);
 

	ComPtr<ID3D12ShaderReflection> reflection;
	DxcBuffer dxcBuffer = {};
	dxcBuffer.Ptr = m_shaderBlob->GetBufferPointer();
	dxcBuffer.Size = m_shaderBlob->GetBufferSize();
	dxcBuffer.Encoding = DXC_CP_ACP; // or DXC_CP_UTF8 if you know it's UTF-8

	ThrowIfFailed(utils->CreateReflection(
		&dxcBuffer, IID_PPV_ARGS(&reflection)
	));


	//std::vector<D3D12_SIGNATURE_PARAMETER_DESC> inputParameters;
 //   std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;

	D3D12_SHADER_DESC shaderDesc{};
	reflection->GetDesc(&shaderDesc);

	switch (shaderDesc.Version >> 16) 
    {
	case D3D12_SHVER_VERTEX_SHADER: {
		this->shaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		std::cout << "shader visibility : VERTEX" << std::endl; // debug output
		break;
	}
	case D3D12_SHVER_PIXEL_SHADER: {
		this->shaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		std::cout << "shader visibility : PIXEL" << std::endl; // debug output
		break;
	}
	case D3D12_SHVER_COMPUTE_SHADER: {
		this->shaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // compute shader
		std::cout << "shader visibility : COMPUTE" << std::endl; // debug output
		break;
	}
		 
	default: {
		std::cerr << "Unknown shader type, version: " << shaderDesc.Version << std::endl;
		return; // or throw an exception
		break;
	}
								  
	}


	// ---- 3.1 input signature  ----------------------------------------------------
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC sig{};
		reflection->GetInputParameterDesc(i, &sig);

		std::cout << "Input Parameter: " << sig.SemanticName
			<< ", Index: " << sig.SemanticIndex
			<< std::endl;

		//compose the actual semantics:
		std::string semantic = std::format("{}{}", sig.SemanticName, sig.SemanticIndex);

		inputParameterMap[semantic] = sig;

		//InputParameter param{};
		//param.semantic = sig.SemanticName;
		//param.semanticIndex = sig.SemanticIndex;
		//param.componentMask = sig.Mask;              // 哪些分量被写
		//param.registerIndex = sig.Register;
		//param.dxgiFormat = DxgiFormatFromMask(sig.Mask, sig.ComponentType);
		//m_inputs.push_back(param);
	}


	// ---- CB -------------------------------------
	for (UINT iCB = 0; iCB < shaderDesc.ConstantBuffers; ++iCB)
	{
		ID3D12ShaderReflectionConstantBuffer* cb =
			reflection->GetConstantBufferByIndex(iCB);

		D3D12_SHADER_BUFFER_DESC cbDesc{};
		cb->GetDesc(&cbDesc);

		//std::cout << "Constant Buffer: " << cbDesc.Name << std::endl;

		//  cb->GetVariableByIndex(j) 
	}


	// ---- 3.3 CBV / SRV / UAV / Sampler ----------------------------------------
	for (UINT iBind = 0; iBind < shaderDesc.BoundResources; ++iBind)
	{
		D3D12_SHADER_INPUT_BIND_DESC bind{};
		reflection->GetResourceBindingDesc(iBind, &bind);

		//std::cout << "Resource Binding: " << bind.Name << std::endl;

		if (bind.Type == D3D_SIT_CBUFFER)
		{
			this->parameterMap.cbvMap[bind.Name] = bind; // CBV
		}
		else if (bind.Type == D3D_SIT_TEXTURE || bind.Type == D3D_SIT_STRUCTURED)
		{
			this->parameterMap.srvMap[bind.Name] = bind; // SRV
		}
		else if (bind.Type == D3D_SIT_UAV_RWTYPED || bind.Type == D3D_SIT_UAV_RWSTRUCTURED)
		{
			this->parameterMap.uavMap[bind.Name] = bind; // UAV
		}
		else if (bind.Type == D3D_SIT_SAMPLER)
		{
			this->parameterMap.samplerMap[bind.Name] = bind; // Sampler
			//continue;
		}

	}
	 
	std::cout << " Shader Reflection Result:" << std::endl;
	std::cout << " Input Parameters: " << inputParameterMap.size() << std::endl;
	std::cout << " SRV number: " << this->parameterMap.srvMap.size() << std::endl;
	std::cout << " CBV number: " << this->parameterMap.cbvMap.size() << std::endl;
	std::cout << " UAV number: " << this->parameterMap.uavMap.size() << std::endl;
	std::cout << " Sampler number: " << this->parameterMap.samplerMap.size() << std::endl;
	 
} 


void FD3D12ShaderPermutation::ReflectRootSignatureLayout(IDxcUtils* dxcUtils)
{
	assert(m_vertexShader != nullptr && m_pixelShader != nullptr);

	this->m_vertexShader->Reflect(dxcUtils);
	this->m_pixelShader->Reflect(dxcUtils); 

	auto& sceneRanges = this->resourceLayout.sceneRanges;
	auto& objectRanges = this->resourceLayout.objectRanges; 
	auto& tableLayout = this->resourceLayout.tableLayout;

	//populate the resource layout by the parameter map:


	UINT localOffset = 0;
	using ResourceMap = std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC>;
	using ResourceBindings = std::unordered_map<std::string, ResourceBinding>;
	auto reflectResType = [&](const ResourceMap& map, ResourceBindings& binding,  D3D12_DESCRIPTOR_RANGE_TYPE rangeType, EShaderStage stage) {
		for (const auto& [name, bind] : map) {
			auto scope = InferScope(name, bind.BindPoint);
			switch (scope) {
			case ResourceScope::Scene:
			{
				sceneRanges.push_back(CD3DX12_DESCRIPTOR_RANGE1(
					rangeType, 1, bind.BindPoint, bind.Space));

				break;
			}

			case ResourceScope::Object:
			{
				objectRanges.push_back(CD3DX12_DESCRIPTOR_RANGE1(
					rangeType, 1, bind.BindPoint, bind.Space )); // space 0 for scene range));
				//fill the table:
				//look for duplicate:
				if (binding.contains(name)) {
					std::cout << "Duplicate binding: " << name << std::endl;
					auto& bindInfo = binding[name];
					if (stage == EShaderStage::VERTEX) bindInfo.visibleVS = true;
					if (stage == EShaderStage::PIXEL) bindInfo.visiblePS = true;
					continue;
				}
				else
				{
					ResourceBinding bindInfo{};
					bindInfo.localOffset = localOffset++;
					if (stage == EShaderStage::VERTEX) bindInfo.visibleVS = true;
					if (stage == EShaderStage::PIXEL) bindInfo.visiblePS = true; 
					binding[name] = bindInfo;
				}

				break;
			}

			default:
				std::cerr << "Unhandled resource scope for SRV: " << name << std::endl;
				break;
			}
		}
		};

	auto reflectStage = [&](const FShaderParameterMap& parms, EShaderStage stage) { 

		reflectResType(parms.srvMap, tableLayout.SRVBindings, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, stage); 
		reflectResType(parms.cbvMap, tableLayout.CBVBindings, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, stage); 
		reflectResType(parms.uavMap, tableLayout.UAVBindings, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, stage);  
		//sampler do nothing; 
		};


	reflectStage(m_vertexShader->parameterMap, EShaderStage::VERTEX);
	reflectStage(m_pixelShader->parameterMap, EShaderStage::PIXEL); 

	tableLayout.tableSize = localOffset;

	std::cout << "Resource Layout Result:" << std::endl;
	std::cout << "Scene Ranges: " << sceneRanges.size() << std::endl;
	std::cout << "Object Ranges: " << objectRanges.size() << std::endl; 
	std::cout << "table size: " << tableLayout.tableSize << std::endl;

	std::cout << "CBV Bindings: " << tableLayout.CBVBindings.size() << std::endl;
	std::cout << "SRV Bindings: " << tableLayout.SRVBindings.size() << std::endl;
	std::cout << "UAV Bindings: " << tableLayout.UAVBindings.size() << std::endl;
	 
}
 
void FD3D12ShaderPermutation::CreateRootSignature()
{
	//convention: 
	// scene range as CBV , slot 0,
	//object range as descriptor table, slot 1;

	//common human error that forget to set static sampler:  
	auto expectedSamplerCount = m_pixelShader->parameterMap.samplerMap.size();
	assert(m_staticSamplers.size() == expectedSamplerCount);



	auto& objectRanges = this->resourceLayout.objectRanges;
	auto& sceneRanges = this->resourceLayout.sceneRanges;
	auto& rootIndexMap = this->resourceLayout.rootIndexMap;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Create the root signature descriptor
	std::vector<D3D12_ROOT_PARAMETER1> rootParameters_; 

	if (!sceneRanges.empty())
	{
		//b0 Scene ranges as root constants
		CD3DX12_ROOT_PARAMETER1 sceneParam{};
		sceneParam.InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters_.push_back(sceneParam);
		rootIndexMap[ResourceScope::Scene] = static_cast<UINT>(rootParameters_.size() - 1);
	}
	
	if (!objectRanges.empty())
	{
		// Object ranges as root descriptor table
		CD3DX12_ROOT_PARAMETER1 objectParam{};
		objectParam.InitAsDescriptorTable(static_cast<UINT>(objectRanges.size()), objectRanges.data(), D3D12_SHADER_VISIBILITY_ALL);
		rootParameters_.push_back(objectParam);
		rootIndexMap[ResourceScope::Object] = static_cast<UINT>(rootParameters_.size() - 1);
	}
	 

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc; 
	rootSignatureDesc.Init_1_1(
		static_cast<UINT>(rootParameters_.size()),
		rootParameters_.data(),
		static_cast<UINT>(m_staticSamplers.size()),
		m_staticSamplers.data(),
		rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	  

}



void FD3D12ShaderPermutation::SetStaticSampler(const std::string& name, const D3D12_STATIC_SAMPLER_DESC& samplerDesc)
{
	if (auto bindPoint = m_pixelShader->GetSamplerBindPoint(name); bindPoint.has_value())
	{
		std::cout << "find static sampler: " << name << " at bind point: " << *bindPoint << std::endl;  

		//check the bind point:
		if (samplerDesc.ShaderRegister != bindPoint.value())
		{
			std::cerr << "Static sampler bind point mismatch: " << name << std::endl; // error output
			return;
		}
		 
		m_staticSamplers.push_back(samplerDesc); 
	} 
	else
	{
		std::cerr << "didn't find static sampler: " << name << std::endl; // error output
		return;
	}
	 


	

}

void FD3D12ShaderPermutation::SetSRV(const std::string& name, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, uint32_t baseOffset)
{
	assert(resource != nullptr && "SRV must not be null"); 

	auto& tableLayout = this->resourceLayout.tableLayout;
	if (tableLayout.SRVBindings.contains(name))
	{
		auto localOffset = tableLayout.SRVBindings[name].localOffset;
		//std::cout << "find texture: " << name << " at offset: " << localOffset << std::endl;  
		auto cpuHandle = m_rangeHeapAllocator.lock()->GetCPUHandle(localOffset + baseOffset);
		m_device->CreateShaderResourceView(resource, &srvDesc, cpuHandle);
	}
	else
	{
		std::cerr << "Failed to bind texture: " << name << std::endl; 
		return;
	}

}


void FD3D12ShaderPermutation::SetCBV(const std::string& name,const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, uint32_t baseOffset)
{
	auto& tableLayout = this->resourceLayout.tableLayout;
	if (tableLayout.CBVBindings.contains(name))
	{
		auto localOffset = tableLayout.CBVBindings[name].localOffset;
		//std::cout << "find CBV: " << name << " at offset: " << localOffset << std::endl;
		auto cpuHandle = m_rangeHeapAllocator.lock()->GetCPUHandle(localOffset + baseOffset);
		m_device->CreateConstantBufferView(&cbvDesc, cpuHandle);
	}
	else
	{
		std::cerr << "Failed to bind CBV: " << name << std::endl;
		return;
	}
}



void FD3D12ShaderPermutation::SetDescriptorHeap(ID3D12GraphicsCommandList* commandList) const
{
	std::vector<ID3D12DescriptorHeap*> ppHeaps = { this->GetDescriptorHeap().Get() };
	commandList->SetDescriptorHeaps(UINT(ppHeaps.size()), ppHeaps.data());
}

void FD3D12ShaderPermutation::SetDescriptorTables(ID3D12GraphicsCommandList* commandList, uint32_t baseOffset) const
{ 
	if (resourceLayout.rootIndexMap.contains(ResourceScope::Object))
	{
		auto objectTableIndex = resourceLayout.rootIndexMap.at(ResourceScope::Object); 
		auto GPUHandle = m_rangeHeapAllocator.lock()->GetGPUHandle(baseOffset);
		commandList->SetGraphicsRootDescriptorTable(objectTableIndex, GPUHandle);
	}
	else
	{
		//std::cerr << "Object descriptor table not found." << std::endl;
	} 
	
}

  

SharedPtr<FD3D12ShaderPermutation> ShaderLibrary::GetOrLoad(const ShaderPermutationKey& key)
{
	//// Prepare macros
	//std::vector<DxcDefine> dxcDefines;
	//for (const auto& def : key.defines) {
	//	dxcDefines.push_back(DxcDefine{ def.c_str(), "1" });
	//}

	// Compile VS
	//ComPtr<IDxcBlob> vsBlob = CompileShader(basePath, "VSMain", dxcDefines);
	//if (!vsBlob) return nullptr;

	//// Compile PS
	//ComPtr<IDxcBlob> psBlob = CompileShader(basePath, "PSMain", dxcDefines);
	//if (!psBlob) return nullptr; 

	if (cache.contains(key))
	{
		std::cout << "Shader permutation found in cache: " << key.shaderTag << ", Pass: " << key.passTag << '\n';
		return cache[key];
	}
		 
	std::string VSBasePath = "shaders/bin/" + key.shaderTag + "_" + key.passTag + "_VS.cso";
	std::string PSBasePath = "shaders/bin/" + key.shaderTag + "_" + key.passTag + "_PS.cso";

	std::cout << "shader not found in cache, " << std::endl;
	std::cout << "try loading VS from: " << VSBasePath << std::endl;
	std::cout << "try loading PS from: " << PSBasePath << std::endl;

	auto fullPathVS = GameApplication::GetInstance()->GetAssetFullPath(VSBasePath);
	auto fullPathPS = GameApplication::GetInstance()->GetAssetFullPath(PSBasePath);
	 
	// Create shader modules
	auto shaderPerm = CreateShared<FD3D12ShaderPermutation>(m_device, m_rangeHeapAllocator);
	shaderPerm->LoadShaders(dxcUtils.Get(), fullPathVS, fullPathPS);
	 
	shaderPerm->ReflectRootSignatureLayout(dxcUtils.Get());

	//shaderPerm->PrepareRootSignature(dxcUtils.Get());
	
	cache[key] = shaderPerm; // Cache the shader permutation
	return shaderPerm; 
}

 