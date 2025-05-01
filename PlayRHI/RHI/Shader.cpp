#include "Shader.h"

#include <fstream>

void FVkShaderModule::ReadSPIRVFile(const std::string& filename)
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


bool FVkShaderModule::CreateShaderModule() {


	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = this->SPIRVBytecode.size() * sizeof(uint32_t);
	createInfo.pCode = this->SPIRVBytecode.data();

	if (vkCreateShaderModule(deviceRef->vkDevice, &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	std::cout << "shader module created" << std::endl;

	return true;
}


void FVkShaderMap::ReflectShaderParameters()
{
	std::cout << "Begin reflect shader parameters" << std::endl;

	//Vk  
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;


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

		// Reflect descriptor bindings 
		uint32_t setCount = 0;
		result = spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, nullptr);
		if (result != SPV_REFLECT_RESULT_SUCCESS) {
			std::cerr << "Failed to enumerate descriptor sets." << std::endl;
			spvReflectDestroyShaderModule(&reflectModule);
			return;
		}

		std::vector<SpvReflectDescriptorSet*> sets(setCount);
		spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, sets.data()); 
		 

		for (uint32_t i = 0; i < setCount; ++i) {

			if(i>0) std::cerr << "we haven't consider more sets." << std::endl;

			std::cout << "reflect set: " << i << std::endl;
			const SpvReflectDescriptorSet& set = *sets[i]; 

			for (uint32_t j = 0; j < set.binding_count; ++j) {
				const SpvReflectDescriptorBinding& binding = *set.bindings[j];

				VkDescriptorSetLayoutBinding layoutBinding{};
				layoutBinding.binding = binding.binding;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = stage; 

				switch (binding.descriptor_type) {
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:


					//engine  
					parameterMap->AddUniformBuffer(
						binding.name,
						FVkShaderParamBufferInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
							.size = binding.block.size,
						});


					//vk

					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					layoutBindings.push_back(layoutBinding);



					std::cout << "Uniform Buffer - dSet: " << set.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << binding.block.size << " bytes"
						<< std::endl;


					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:

					//engine  
					parameterMap->AddStorageBuffer(
						binding.name,
						FVkShaderParamBufferInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
							.size = binding.block.size,
						});

					//vk

					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					layoutBindings.push_back(layoutBinding);


					std::cout << "Storage Buffer - dSet: " << set.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name
						<< ",block size: " << binding.block.size << " bytes"
						<< std::endl;

					break;

				case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: 

					//reflect:
					parameterMap->AddSampler(
						binding.name,
						FVkShaderParamTextureInfo{
							.stageFlags = stage,
							.binding = binding.binding,
							.set = binding.set,
						});
					 
					//vk 
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					layoutBindings.push_back(layoutBinding);


					std::cout << "Sampler - dSet: " << set.set
						<< ", Binding: " << binding.binding
						<< ",name: " << binding.name << std::endl;

					break;
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
				FVkShaderParamPushConstantInfo pushConstantInfo{};
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



	}//stage 




	//vk
	std::cout << "push constant range created, length: " << pushConstantRanges.size() << std::endl;


	//vk
	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(deviceRef->vkDevice, &layoutInfo, nullptr, &this->descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	std::cout << "descriptor set layout created,  length: " << layoutBindings.size() << std::endl;




}//function