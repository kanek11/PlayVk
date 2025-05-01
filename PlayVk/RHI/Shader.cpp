#include "Shader.h"

#include <fstream>

//todo: may be fold the duplicate code? 


void FVkShaderMap::bindUniformBuffer(const std::string& name, const FVkBufferRTInfo& rtInfo)
{
	auto& device = deviceRef.lock()->vkDevice;

	// Check if the storage buffer is present in the map
	auto& uboMap = parameterMap->uniformBufferMap;
	if (!uboMap.contains(name))
	{
		throw std::runtime_error("Shader: Uniform buffer not found: " + name);
		std::cerr << "Shader: Storage buffer not found: " << name << '\n';
		return;
	}

	auto& bindInfo = uboMap[name];
	bindInfo.rtInfo = rtInfo; 
	
	//std::cout << "reflect: bind ubo :" << name << '\n';
	//std::cout << "reflect: bound to: set: " << bindInfo.set << ", binding: " << bindInfo.binding << '\n'; 

	updateUBOCache.push_back(bindInfo);
 
}




void FVkShaderMap::bindStorageBuffer(const std::string& name, const FVkBufferRTInfo& rtInfo)
{
	auto& device = deviceRef.lock()->vkDevice; 

	// Check if the storage buffer is present in the map
	auto& storageBufferMap = parameterMap->storageBufferMap;
	if (!storageBufferMap.contains(name))
	{
		throw std::runtime_error("Shader: Storage buffer not found: " + name);
		std::cerr << "Shader: Storage buffer not found: " << name << '\n';
		return;
	}  

	auto& bindInfo = storageBufferMap[name]; 
	bindInfo.rtInfo = rtInfo;

	//std::cout << "reflect: bind ssbo :" << name << '\n';
	//std::cout << "reflect: bound to: set: " << bindInfo.set << ", binding: " << bindInfo.binding << '\n';
	
	updateSSBOCache.push_back(bindInfo);
	
}

void FVkShaderMap::bindTexture(const std::string& name, const FVkTextureRTInfo& rtInfo)
{ 

	// Check if the storage buffer is present in the map
	auto& samplerMap = parameterMap->samplerMap;
	if (!samplerMap.contains(name))
	{
		throw std::runtime_error("Shader: Texture not found: " + name);
		std::cerr << "Shader: Texture not found: " << name << '\n';
		return;
	}
	//std::cout << "reflect: bind texture :" << name << '\n';
	//std::cout << "reflect: bound to: set: " << bindInfo.set << ", binding: " << bindInfo.binding << '\n'; 

	auto& bindInfo = samplerMap[name];  
	bindInfo.rtInfo = rtInfo;

	updateTextureCache.push_back(bindInfo);

}



void FVkShaderMap::updateBindings(std::vector<VkDescriptorSet>& dSet)
{ 
	auto& device = deviceRef.lock()->vkDevice;  

	//validate:
	if (dSet.empty()) {
		std::cerr << "Shader: descriptor set was not allocated!" << '\n';
		return;
	}  

	std::vector<VkWriteDescriptorSet> writes;

	//eliminite undefined behavior cause the infos out of scope; 
	//Reserve to prevent reallocation
	std::vector<VkDescriptorBufferInfo> uboInfos;
	uboInfos.reserve(updateUBOCache.size()); 

	std::vector<VkDescriptorBufferInfo> ssboInfos;
	ssboInfos.reserve(updateSSBOCache.size());

	std::vector<VkDescriptorImageInfo> imageInfos;
	imageInfos.reserve(updateTextureCache.size());   

	for (auto& bindInfo : updateUBOCache)
	{
		if (!bindInfo.rtInfo.has_value()) {
			std::cerr << "Shader: resource not bound: " << bindInfo.set << "," << bindInfo.binding << '\n';
			continue;
		}

		uboInfos.emplace_back();
		VkDescriptorBufferInfo& bufferInfo = uboInfos.back();
		bufferInfo.buffer = bindInfo.rtInfo->buffer;
		bufferInfo.offset = bindInfo.rtInfo->offset;
		bufferInfo.range = bindInfo.rtInfo->range;

		VkWriteDescriptorSet descriptorWrite{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = dSet[bindInfo.set];
		descriptorWrite.dstBinding = bindInfo.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		writes.push_back(descriptorWrite);
	}
	
	for (auto& bindInfo : updateSSBOCache)
	{
		if (!bindInfo.rtInfo.has_value()) {
			std::cerr << "Shader: resource not bound: " << bindInfo.set << "," << bindInfo.binding << '\n';
			continue;
		}
		
		ssboInfos.emplace_back();
		VkDescriptorBufferInfo& bufferInfo = ssboInfos.back(); 
		bufferInfo.buffer = bindInfo.rtInfo->buffer;
		bufferInfo.offset = bindInfo.rtInfo->offset;
		bufferInfo.range = bindInfo.rtInfo->range;

		VkWriteDescriptorSet descriptorWrite{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = dSet[bindInfo.set];
		descriptorWrite.dstBinding = bindInfo.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		writes.push_back(descriptorWrite);
	}
	
	for (auto& bindInfo : updateTextureCache)
	{
		if (!bindInfo.rtInfo.has_value()) {
			std::cerr << "Shader: resource not bound: " << bindInfo.set << "," << bindInfo.binding << '\n';
			continue;
		} 

		imageInfos.emplace_back();
		VkDescriptorImageInfo& imageInfo = imageInfos.back();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = bindInfo.rtInfo->imageView;
		imageInfo.sampler = bindInfo.rtInfo->sampler;

		VkWriteDescriptorSet descriptorWrite{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = dSet[bindInfo.set];
		descriptorWrite.dstBinding = bindInfo.binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		writes.push_back(descriptorWrite);
	}

	//std::cout << "update descriptor num£º" << writes.size() << '\n';
	  
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr); 
 

	updateUBOCache.clear();
	updateSSBOCache.clear();
	updateTextureCache.clear();
}


bool FVkShaderMap::validateBindings() const
{  
	bool valid = true; 

		auto& storageBufferMap = parameterMap->storageBufferMap;
		auto& uniformBufferMap = parameterMap->uniformBufferMap;
		auto& samplerMap = parameterMap->samplerMap; 

		for (auto& [name, bindInfo] : uniformBufferMap)
		{
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				valid = false;
				continue;
			}
		}


		for (auto& [name, bindInfo] : storageBufferMap) 
		{ 
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				valid = false;
				continue;
			}

		} 

		for (auto& [name, bindInfo] : samplerMap)
		{
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				valid = false;
				continue;
			}
		} 

	return valid;
 
}




/*
the out of memory behavior for pooling, is implementation-dependent,
it might not be as strict ;
*/ 

void FVkShaderMap::createDescriptorPool() {

	auto& device = deviceRef.lock()->vkDevice;

	std::vector<VkDescriptorPoolSize> poolSizes{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 } ,

	};

	VkDescriptorPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 100;

	VkDescriptorPool _descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	 this->descriptorPool = _descriptorPool; 

	std::cout << "allocated d pool" << '\n';
}



void FVkShaderMap::allocateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets) const
{
	auto& device = deviceRef.lock()->vkDevice;

	//all layouts in a single allocation must be unique  without extension support
	descriptorSets.resize(descriptorSetLayouts.size());

	for (uint32_t j = 0; j < descriptorSetLayouts.size(); ++j) {  
		VkDescriptorSetAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = this->descriptorPool; 
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayouts[j];

		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[j]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		} 
	}

	std::cout << "allocated dsets" << '\n';
}




void FVkShaderModule::readSPIRVFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	// size in bytes
	size_t fileSize = static_cast<size_t>(file.tellg());

	// SPIR-V is 32-bit aligned
	if (fileSize % sizeof(uint32_t) != 0) {
		throw std::runtime_error("SPIR-V file size is not aligned to 32-bit word size.");
	}

	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	file.close();

	this->SPIRVBytecode = buffer;
}


bool FVkShaderModule::createShaderModule() {

	auto& device = deviceRef.lock()->vkDevice;

	VkShaderModuleCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = static_cast<uint32_t> (this->SPIRVBytecode.size() * sizeof(uint32_t));
	createInfo.pCode = this->SPIRVBytecode.data();

	if (vkCreateShaderModule(device, &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	std::cout << "shader module created" << '\n'; 
	return true;
}


void FVkShaderMap::reflectShaderParameters()
{
	std::cout << "Begin reflect shader parameters" << '\n'; 
	

	//dim1: sets, dim2: bindings, map is for clarity;
	std::vector< std::vector<VkDescriptorSetLayoutBinding> > layoutBindings;
	 
	for (const auto& [stage, shaderModule] : shaderModuleMap)
	{
		std::vector<uint32_t>& spirvCode = shaderModule->SPIRVBytecode;

		// Initialize SPIRV-Reflect
		SpvReflectShaderModule reflectModule;
		SpvReflectResult result = spvReflectCreateShaderModule(shaderModule->SPIRVBytecode.size() * sizeof(uint32_t), shaderModule->SPIRVBytecode.data(), &reflectModule);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			std::cerr << "Failed to create SPIRV-Reflect module." << '\n';
			return;
		}

		uint32_t localSetCount = 0;
		//descriptor bindings  
		result = spvReflectEnumerateDescriptorSets(&reflectModule, &localSetCount, nullptr);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			std::cerr << "Failed to enumerate descriptor sets." << '\n';
			spvReflectDestroyShaderModule(&reflectModule);
			return;
		}

		std::vector<SpvReflectDescriptorSet*> sets(localSetCount);
		spvReflectEnumerateDescriptorSets(&reflectModule, &localSetCount, sets.data()); 
		  

		//caveat: binding.set and this index is local to the module!
		for (uint32_t i = 0; i < localSetCount; ++i) {  

			const SpvReflectDescriptorSet& thisSet = *sets[i]; 

			for (uint32_t j = 0; j < thisSet.binding_count; ++j) {
				const SpvReflectDescriptorBinding& binding = *thisSet.bindings[j];
				 
				std::cout << "stage: reflect set index: " << thisSet.set << '\n';
				if (layoutBindings.size() < thisSet.set + 1)
				{
					layoutBindings.resize(thisSet.set + 1);
				}

				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding = binding.binding;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = stage; 

				switch (binding.descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: 

					//engine  
					parameterMap->AddUniformBuffer(
						binding.name,
						FVkShaderResBufferInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = thisSet.set,
							.size = binding.block.size,
						});


					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					layoutBindings[thisSet.set].push_back(layoutBinding);
					 

					std::cout << "Uniform Buffer - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << binding.block.size << " bytes"
						<< '\n'; 

					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:

					//engine  
					parameterMap->AddStorageBuffer(
						binding.name,
						FVkShaderResBufferInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = thisSet.set,
							.size = binding.block.size,
						});

					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					layoutBindings[thisSet.set].push_back(layoutBinding);


					std::cout << "Storage Buffer - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << "unknown for dynamic array in std430" 
						<< '\n';

					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: 

					//reflect:
					parameterMap->AddSampler(
						binding.name,
						FVkShaderResTextureInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = thisSet.set,
						});
					 
					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					layoutBindings[thisSet.set].push_back(layoutBinding);


					std::cout << "Sampler - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name 
						<< '\n';


					break;

				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: 

					//engine:
					parameterMap->AddStorageImage(
						binding.name,
						FVkShaderResTextureInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = thisSet.set,
						});


					 //vk:
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					layoutBindings[thisSet.set].push_back(layoutBinding);


					std::cout << "image storage - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name 
						<< '\n';

					break;


					//others
				default:
					std::cerr << "we haven't consider the shader input yet." << '\n';
				}

			}//bindings

		} //set

		

		//push constants  

		uint32_t pushConstantCount = 0;
		result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, nullptr);

		if (result == SPV_REFLECT_RESULT_SUCCESS && pushConstantCount > 0) {
			std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
			spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, pushConstants.data());


			for (uint32_t i = 0; i < pushConstantCount; ++i) {
				const SpvReflectBlockVariable& block = *pushConstants[i];

				//engine 
				FVkShaderResPushConstantInfo pushConstantInfo{};
				pushConstantInfo.stageFlags = stage;
				pushConstantInfo.size = block.size;

				parameterMap->AddPushConstant(block.name, pushConstantInfo);


				//vk
				VkPushConstantRange pushConstantRange{};
				pushConstantRange.stageFlags = stage;
				pushConstantRange.offset = block.offset;
				pushConstantRange.size = block.size;

				this->pushConstantRanges.push_back(pushConstantRange);


				//debug
				auto result = std::format("push constant, name: {0} created -  size: {1} bytes, offset{2}", block.name, block.size, block.offset);
				std::cout << result << '\n';

				for (uint32_t j = 0; j < block.member_count; ++j) {
					const SpvReflectBlockVariable& member = block.members[j];
					std::cout << "  Member name: " << member.name << ", Offset: " << member.offset << '\n';
				} 
			} 
		} 
		 

		//vertex input for vertex shader
		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			uint32_t inputCount = 0;
			result = spvReflectEnumerateInputVariables(&reflectModule, &inputCount, nullptr);

			if (result == SPV_REFLECT_RESULT_SUCCESS && inputCount > 0) {
				std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);
				spvReflectEnumerateInputVariables(&reflectModule, &inputCount, inputs.data());

				for (uint32_t i = 0; i < inputCount; ++i) {
					const SpvReflectInterfaceVariable& input = *inputs[i];

					//engine

					FVkShaderParamVertexInputInfo inputInfo{};
					inputInfo.location = input.location;

					vertexInputMap[input.name] = inputInfo;


					std::cout
						<< "Input Variable - Location: " << input.location
						<< ",name: " << input.name << '\n';
				}
			}
		}



	}//all stage 
	 
	std::cout << "push constant range total: " << pushConstantRanges.size() << '\n';


	auto& device = deviceRef.lock()->vkDevice; 
	descriptorSetLayouts.resize(layoutBindings.size()); //
	// 
	uint32_t set = 0;
	for (auto& bindings : layoutBindings)
	{

		VkDescriptorSetLayoutCreateInfo layoutInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VkDescriptorSetLayout descriptorSetLayout;
		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		std::cout << "dset layout created for set:" << set << '\n';
		std::cout << "bingings num : " << bindings.size() << '\n'; 
		 
		descriptorSetLayouts[set] = descriptorSetLayout;

		set++;
	}
	  
	std::cout << "descriptor set layouts final set number: " << descriptorSetLayouts.size() << '\n';
	 



	//todo: make this more explicit? ;driven by reflection result?
	createDescriptorPool();


}//function