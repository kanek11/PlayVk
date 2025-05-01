#include "Shader.h"

#include <fstream>


/*
the out of memory behavior for pooling, is implementation-dependent,
it might not be as strict ;
*/ 

void FVkShaderMap::createDescriptorPool() {

	auto device = deviceRef->vkDevice;

	std::vector<VkDescriptorPoolSize> poolSizes{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 } ,

	};

	VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 10;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	std::cout << "allocated d pool" << std::endl;
}



void FVkShaderMap::allocateDescriptorSets()
{
	auto device = deviceRef->vkDevice;

	//all layouts in a single allocation must be unique 
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		descriptorSets[i].resize(descriptorSetLayouts.size());

		for (size_t j = 0; j < descriptorSetLayouts.size(); ++j) {
			VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &descriptorSetLayouts[j];

			if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[i][j]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor set!");
			}
		}
	}

	std::cout << "allocated dsets" << std::endl;
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


	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = static_cast<uint32_t> (this->SPIRVBytecode.size() * sizeof(uint32_t));
	createInfo.pCode = this->SPIRVBytecode.data();

	if (vkCreateShaderModule(deviceRef->vkDevice, &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	std::cout << "shader module created" << std::endl;

	return true;
}


void FVkShaderMap::reflectShaderParameters()
{
	std::cout << "Begin reflect shader parameters" << std::endl; 
	

	//Vk  
	std::vector< std::vector<VkDescriptorSetLayoutBinding> >layoutBindings;


	for (const auto& [stage, shaderModule] : shaderModuleMap)
	{
		std::vector<uint32_t>& spirvCode = shaderModule->SPIRVBytecode;

		// Initialize SPIRV-Reflect
		SpvReflectShaderModule reflectModule;
		SpvReflectResult result = spvReflectCreateShaderModule(shaderModule->SPIRVBytecode.size() * sizeof(uint32_t), shaderModule->SPIRVBytecode.data(), &reflectModule);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			std::cerr << "Failed to create SPIRV-Reflect module." << std::endl;
			return;
		}

		uint32_t setCount = 0;
		//descriptor bindings  
		result = spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, nullptr);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			std::cerr << "Failed to enumerate descriptor sets." << std::endl;
			spvReflectDestroyShaderModule(&reflectModule);
			return;
		}

		std::vector<SpvReflectDescriptorSet*> sets(setCount);
		spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, sets.data()); 
		 
		//resize the containers
		if(setCount > layoutBindings.size())
		layoutBindings.resize(setCount); 


		for (uint32_t i = 0; i < setCount; ++i) { 
		/*	if (i > 0)
			{
				std::cerr << "we haven't consider more sets." << std::endl;
				break;
			}*/ 

			std::cout << "stage: reflect set: " << i << std::endl;

			const SpvReflectDescriptorSet& thisSet = *sets[i]; 

			for (uint32_t j = 0; j < thisSet.binding_count; ++j) {
				const SpvReflectDescriptorBinding& binding = *thisSet.bindings[j];

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
							.set = binding.set,
							.size = binding.block.size,
						});


					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					layoutBindings[i].push_back(layoutBinding);
					 

					std::cout << "Uniform Buffer - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << binding.block.size << " bytes"
						<< std::endl; 

					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:

					//engine  
					parameterMap->AddStorageBuffer(
						binding.name,
						FVkShaderResBufferInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
							.size = binding.block.size,
						});

					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					layoutBindings[i].push_back(layoutBinding);


					std::cout << "Storage Buffer - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << binding.block.size << " bytes"
						<< std::endl;

					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: 

					//reflect:
					parameterMap->AddSampler(
						binding.name,
						FVkShaderResTextureInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
						});
					 
					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					layoutBindings[i].push_back(layoutBinding);


					std::cout << "Sampler - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name 
						<< std::endl;


					break;

				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: 

					//engine:
					parameterMap->AddStorageImage(
						binding.name,
						FVkShaderResTextureInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
						});


					 //vk:
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					layoutBindings[i].push_back(layoutBinding);


					std::cout << "image storage - dSet: " << thisSet.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name 
						<< std::endl;

					break;


					//others
				default:
					std::cerr << "we haven't consider the shader input yet." << std::endl;
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
				std::cout << "Push Constant - Size: " << block.size << " bytes"
					<< ",name: " << block.name << std::endl;

				for (uint32_t j = 0; j < block.member_count; ++j) {
					const SpvReflectBlockVariable& member = block.members[j];
					std::cout << "  Member name: " << member.name << ", Offset: " << member.offset << std::endl;
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
						<< ",name: " << input.name << std::endl;
				}
			}
		}



	}//all stage 

	
	std::cout << "push constant range created, length: " << pushConstantRanges.size() << std::endl;


	for (size_t i = 0; i < layoutBindings.size(); i++)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings[i].size());
		layoutInfo.pBindings = layoutBindings[i].data();

		VkDescriptorSetLayout descriptorSetLayout;
		if (vkCreateDescriptorSetLayout(deviceRef->vkDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		std::cout << "descriptor set layout created, number of binding: " << layoutBindings[i].size() << std::endl;
		this->descriptorSetLayouts.push_back(descriptorSetLayout);
	}
	  
	std::cout << "descriptor set layouts final set number: " << descriptorSetLayouts.size() << std::endl;
	 




	createDescriptorPool();
	allocateDescriptorSets();


}//function