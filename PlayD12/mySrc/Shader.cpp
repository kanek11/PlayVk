#include "Shader.h"

#include <format>

void FD3D12ShaderModule::CreateDescriptorTable(SharedPtr<FDescriptorHeapAllocator> allocator)
{ 
	uint32_t heapStartOffset = allocator->GetCurrentOffset(); 

	uint32_t localOffset = 0;
	//---------------------
	std::unordered_map<std::string, uint32_t> SRVMemberOffsets;
	for (const auto& [name, bind] : this->parameterMap.srvMap)
	{
		CD3DX12_DESCRIPTOR_RANGE1 range{};
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, bind.BindPoint, bind.Space,
			D3D12_DESCRIPTOR_RANGE_FLAG_NONE, localOffset);
		ranges.push_back(range); 

		SRVMemberOffsets[name] = localOffset; 
		std::cout << "Register SRV member: " << name << " local offset: " << localOffset << std::endl;

		localOffset += 1;
	}

	FD3D12BindingLayout srvLayout{};
	srvLayout.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvLayout.numDescriptors = (UINT)parameterMap.srvMap.size();
	srvLayout.baseRegister = 0;
	srvLayout.heapOffset = allocator->Allocate(srvLayout.numDescriptors); 

	std::cout << "SRV Descriptor number : " << srvLayout.numDescriptors << std::endl; //debug output


	//--------------------- 
	std::unordered_map<std::string, uint32_t> CBVMemberOffsets;  
	for (const auto& [name, bind] : this->parameterMap.cbvMap)
	{
		CD3DX12_DESCRIPTOR_RANGE1 range{};
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, bind.BindPoint, bind.Space,
			D3D12_DESCRIPTOR_RANGE_FLAG_NONE, localOffset);
		ranges.push_back(range);  

		CBVMemberOffsets[name] = localOffset;
		std::cout << "Register CBV member: " << name << " local offset: " << localOffset << std::endl; //debug output
		 
		localOffset += 1;
	}

	FD3D12BindingLayout cbvLayout{};
	cbvLayout.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	cbvLayout.numDescriptors = (UINT)parameterMap.cbvMap.size();
	cbvLayout.baseRegister = 0;
	cbvLayout.heapOffset = allocator->Allocate(cbvLayout.numDescriptors);
	std::cout << "CBV Descriptor number : " << cbvLayout.numDescriptors << std::endl; //debug output


	//--------------------- 
	std::unordered_map<std::string, uint32_t> UAVMemberOffsets;
	for (const auto& [name, bind] : this->parameterMap.uavMap)
	{
		CD3DX12_DESCRIPTOR_RANGE1 range{};
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, bind.BindPoint, bind.Space,
			D3D12_DESCRIPTOR_RANGE_FLAG_NONE, localOffset);
		ranges.push_back(range);   

		UAVMemberOffsets[name] = localOffset;
		std::cout << "Register UAV member: " << name << " local offset: " << localOffset << std::endl; //debug output

		localOffset += 1;
	}


	FD3D12BindingLayout uavLayout{};
	uavLayout.type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavLayout.numDescriptors = (UINT)parameterMap.uavMap.size();
	uavLayout.baseRegister = 0;
	uavLayout.heapOffset = allocator->Allocate(uavLayout.numDescriptors);
	std::cout << "UAV Descriptor number : " << uavLayout.numDescriptors << std::endl; //debug output
	
	//---------------------
	//the sampler: 
	//for (const auto& [name, bind] : this->parameterMap.samplerMap)
	//{
	//	CD3DX12_DESCRIPTOR_RANGE1 range{};
	//	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, bind.BindPoint, bind.Space);
	//	ranges.push_back(range);
	//} 

	
	//dynamic samplers are not considered;
	//FDescriptorTableLayout samplerLayout{};
	//samplerLayout.type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	//samplerLayout.numDescriptors = (UINT)sampler_ranges.size();
	//samplerLayout.baseRegister = 0;
	//samplerLayout.heapOffset = allocator->Allocate(samplerLayout.numDescriptors);
	//std::cout << "Sampler Descriptor Table start at: " << samplerLayout.heapOffset << std::endl; //debug output
	 
	CD3DX12_ROOT_PARAMETER1 combinedTable{};
	combinedTable.InitAsDescriptorTable(
		static_cast<UINT>(ranges.size()), ranges.data(), this->shaderVisibility
	);

	std::cout << "Create Combined Descriptor Table of size: " << 
		std::format("heap offset: {}, table size: {}", heapStartOffset, localOffset) << std::endl; // debug output

	this->descriptorTable = combinedTable;  
	 
	this->tableLayout.heapOffset = heapStartOffset; 
	this->tableLayout.tableSize = localOffset;  

	this->tableLayout.SRVLayout = srvLayout;
	this->tableLayout.SRVMemberOffsets = SRVMemberOffsets;
	this->tableLayout.CBVLayout = cbvLayout;
	this->tableLayout.CBVMemberOffsets = CBVMemberOffsets;
	this->tableLayout.UAVLayout = uavLayout;
	this->tableLayout.UAVMemberOffsets = UAVMemberOffsets;
	//this->tableLayouts.samplerLayout = samplerLayout;
}

void FD3D12ShaderModule::Reflect(IDxcUtils* utils)
{

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

	switch (shaderDesc.Version >> 16) {
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

		// ...
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

        std::cout << "Constant Buffer: " << cbDesc.Name
            << std::endl;

        //  cb->GetVariableByIndex(j) 
    }


    // ---- 3.3 CBV / SRV / UAV / Sampler ----------------------------------------
    for (UINT iBind = 0; iBind < shaderDesc.BoundResources; ++iBind)
    {
        D3D12_SHADER_INPUT_BIND_DESC bind{};
        reflection->GetResourceBindingDesc(iBind, &bind);
         
        std::cout << "Resource Binding: " << bind.Name << std::endl;
		 
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
	 

	std::cout << "Shader Reflection Result:" << std::endl;
	std::cout << " Input Parameters: " << inputParameterMap.size() << std::endl;
	std::cout << " SRV number: " << this->parameterMap.srvMap.size() << std::endl;
	std::cout << " CBV number: " << this->parameterMap.cbvMap.size() << std::endl;
	std::cout << " UAV number: " << this->parameterMap.uavMap.size() << std::endl;
	std::cout << " Sampler number: " << this->parameterMap.samplerMap.size() << std::endl;

     
}

void FD3D12GraphicsShaderManager::Init()
{ 
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));

}

void FD3D12GraphicsShaderManager::PrepareRootSignature()
{ 
	this->m_vertexShader->Reflect(dxcUtils.Get()); 
	this->m_pixelShader->Reflect(dxcUtils.Get());  

	//get the size of the descriptor table:
	auto VSParameterCount = m_vertexShader->GetParameterCount();
	auto PSParameterCount = m_pixelShader->GetParameterCount();

	auto totalCount = VSParameterCount + PSParameterCount;

	//create heap by the count:
	m_rangeHeapAllocator = CreateShared<FDescriptorHeapAllocator>(
		m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, totalCount
	);

	std::cout << "Created range heap of size: " << totalCount << std::endl; // debug output
	 
	//create the descriptor tables:
	m_vertexShader->CreateDescriptorTable(m_rangeHeapAllocator);
	m_pixelShader->CreateDescriptorTable(m_rangeHeapAllocator);

	m_rootSignatureLayout.numTables = 2; // Two descriptor tables: one for vertex shader, one for pixel shader
	m_rootSignatureLayout.tableIndexMap[EShaderStage::VERTEX] = 0; // Vertex Shader Table Index
	m_rootSignatureLayout.tableIndexMap[EShaderStage::PIXEL] = 1; // Pixel Shader Table Index
}



void FD3D12GraphicsShaderManager::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	//CD3DX12_DESCRIPTOR_RANGE1 ranges[2]{};
	//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);  // b0
	//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // t0

	//CD3DX12_ROOT_PARAMETER1 rootParameters[1]{};
	//rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

	auto numTables = m_rootSignatureLayout.numTables;
	auto vsTableIndex = m_rootSignatureLayout.tableIndexMap[EShaderStage::VERTEX];
	auto psTableIndex = m_rootSignatureLayout.tableIndexMap[EShaderStage::PIXEL];

	std::vector<D3D12_ROOT_PARAMETER1> rootParameters(numTables); // Create a vector to hold root parameters
	//rootParameters.push_back(m_vertexShader->descriptorTable);
	//rootParameters.push_back(m_pixelShader->descriptorTable);
	rootParameters[vsTableIndex] = m_vertexShader->descriptorTable; // Vertex Shader Descriptor Table
	rootParameters[psTableIndex] = m_pixelShader->descriptorTable; // Pixel Shader Descriptor Table

	std::cout << "Root Parameters Count: " << rootParameters.size() << std::endl; // Debug output


	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; 


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
	//rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);
	rootSignatureDesc.Init_1_1(
		static_cast<UINT>(rootParameters.size()),
		rootParameters.data(),
		static_cast<UINT>(m_staticSamplers.size()),
		m_staticSamplers.data(),
		rootSignatureFlags); 

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void FD3D12GraphicsShaderManager::SetSRV(const std::string& name, ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	if (auto offset = m_pixelShader->GetHeapOffsetSRV("baseMap"); offset.has_value())
	{
		std::cout << "find texture: " << name << " at offset: " << *offset << std::endl; // debug output
		auto cpuHandle = m_rangeHeapAllocator->GetCPUHandle(*offset);
		m_device->CreateShaderResourceView(resource.Get(), &srvDesc, cpuHandle);
	}
	else
	{
		std::cerr << "Failed to bind texture: " << name << std::endl; // error output
		return;
	}

}

void FD3D12GraphicsShaderManager::bindStaticSampler(const std::string& name, const D3D12_STATIC_SAMPLER_DESC& samplerDesc)
{
	if (m_pixelShader->HasSampler(name))
	{
		m_staticSamplers.push_back(samplerDesc);
	}
	else
	{
		std::cerr << "Failed to bind static sampler: " << name << std::endl; // error output
		return;
	}
	
}


void FD3D12GraphicsShaderManager::SetCBV(const std::string& name, ComPtr<ID3D12Resource> resource, const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc)
{
	std::optional<uint32_t> offset;
	if (offset = m_vertexShader->GetHeapOffsetCBV(name); offset.has_value())
	{
		std::cout << "find constant buffer visible to VS: " << name << " at offset: " << *offset << std::endl; // debug output

	}
	else if (offset = m_pixelShader->GetHeapOffsetCBV(name); offset.has_value())
	{
		std::cout << "find constant buffer visible to PS: " << name << " at offset: " << *offset << std::endl; // debug output
	}

	if (offset.has_value())
	{
		auto cpuHandle = m_rangeHeapAllocator->GetCPUHandle(*offset);
		m_device->CreateConstantBufferView(&cbvDesc, cpuHandle);
	}
	else
	{
		std::cerr << "Failed to bind constant buffer: " << name << std::endl; // error output
		return;
	}
}



void FD3D12GraphicsShaderManager::SetDescriptorHeap(ComPtr<ID3D12GraphicsCommandList> commandList) const
{
	std::vector<ID3D12DescriptorHeap*> ppHeaps = { this->GetDescriptorHeap().Get() };
	commandList->SetDescriptorHeaps(UINT(ppHeaps.size()), ppHeaps.data());

}

void FD3D12GraphicsShaderManager::SetAllDescriptorTables(ComPtr<ID3D12GraphicsCommandList> commandList) const
{

	//m_commandList->SetGraphicsRootDescriptorTable(0, m_cbv_srv_heap->GetGPUDescriptorHandleForHeapStart());
	auto vsTableHeapOffset = m_vertexShader->GetHeapStartOffset();
	auto psTableHeapOffset = m_pixelShader->GetHeapStartOffset();

	auto VSGPUHandle = m_rangeHeapAllocator->GetGPUHandle(vsTableHeapOffset);
	auto PSGPUHandle = m_rangeHeapAllocator->GetGPUHandle(psTableHeapOffset);

	auto vsTableIndex = m_rootSignatureLayout.tableIndexMap.at(EShaderStage::VERTEX);
	auto psTableIndex = m_rootSignatureLayout.tableIndexMap.at(EShaderStage::PIXEL);

	commandList->SetGraphicsRootDescriptorTable(vsTableIndex, VSGPUHandle);
	commandList->SetGraphicsRootDescriptorTable(psTableIndex, PSGPUHandle);
}