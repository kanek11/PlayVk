#pragma once

#include <vector>
#include <array>
#include <unordered_map>


#include <vulkan/vulkan.h>
#include <spirv_reflect.h>

#include "Base.h"
#include "GVulkanRHI.h"

//todo : runtime validation, state track check if every binding point is set;

//handle multiple descriptor sets;


		//array? VkPipelineShaderStageCreateInfo shaderStageInfo; 
/*
*

* by SPIR-V standard,
define 5 fundamental ways to map data from the host to the shader;
descriptor set
push constant
sampler
Physical Storage Buffer
Specialization Constants
*
* ParameterMap  stores the reflection information of shader parameters ,
*
* when actual binding,
* the actual metainfo carried on the engine object is used to create []Info
* lookup names on the map, retrieve the binding point and set index ,  instead of do it manually;
*
*
* informations that is not directly consumed for binding,
* eg: the reflected size ,  can be used to validate the bound buffer size as engine-side validation;

*/

//simple for now so the mesh doens't need to know the shader layout;
struct FVkShaderParamVertexInputInfo
{
	uint32_t location;
};




/*
bindable resources
*/
struct FVkShaderResBufferInfo
{
	//dset layout info  
	VkShaderStageFlags   stageFlags;
	uint32_t             binding; // or BaseIndex; location; 

	uint32_t  	         set; //  set it belongs to;
	//data info
	uint32_t             size;    // or range; 
};

struct FVkShaderResTextureInfo
{
	VkShaderStageFlags   stageFlags;
	uint32_t             binding;
	uint32_t  	         set;
	//image is seen as a immutable whole; so no size or offset;
};


//push constant is unique per , no binding point
struct FVkShaderResPushConstantInfo
{
	VkShaderStageFlags   stageFlags;
	uint32_t             size;
};


/*
* static metadata from shader reflection;

*/
class FVkShaderParamMap
{
public:
	using UniformBufferMap = std::unordered_map<std::string, FVkShaderResBufferInfo>;
	using StorageBufferMap = std::unordered_map<std::string, FVkShaderResBufferInfo>;
	using SamplerMap = std::unordered_map<std::string, FVkShaderResTextureInfo>;
	using StorageImageMap = std::unordered_map<std::string, FVkShaderResTextureInfo>;

	using PushConstantMap = std::unordered_map<std::string, FVkShaderResPushConstantInfo>;



	// API for the reflection process , use const char* to be compatible with SPIR-V reflection
public:
	void AddSampler(const char* name, const FVkShaderResTextureInfo& info)
	{
		samplerMap.emplace(std::string(name), info);
	}

	void AddUniformBuffer(const char* name, const FVkShaderResBufferInfo& info)
	{
		uniformBufferMap.emplace(std::string(name), info);
	}

	void AddStorageBuffer(const char* name, const FVkShaderResBufferInfo& info)
	{
		storageBufferMap.emplace(std::string(name), info);
	}

	void AddPushConstant(const char* name, const FVkShaderResPushConstantInfo& info)
	{
		pushConstantMap.emplace(std::string(name), info);
	}
	
	void AddStorageImage(const char* name, const FVkShaderResTextureInfo& info)
	{
		storageImageMap.emplace(std::string(name), info);
	}

public:
	SamplerMap       samplerMap;
	UniformBufferMap uniformBufferMap;
	StorageBufferMap storageBufferMap;
	PushConstantMap  pushConstantMap;
	StorageImageMap storageImageMap;
};

class FVkShaderModule
{
public:
	FVkShaderModule(SharedPtr<FVkDevice> device, VkShaderStageFlags stage, const std::string& filepath) :
		deviceRef(device), stage(stage)
	{
		readSPIRVFile(filepath);
		createShaderModule();
	}

	bool createShaderModule();
	void readSPIRVFile(const std::string& filepath);



public:
	std::vector<uint32_t> SPIRVBytecode;
	VkShaderModule shaderModule = VK_NULL_HANDLE;


private:
	const VkShaderStageFlags stage;
	const SharedPtr<FVkDevice> deviceRef;
};


/*
 * per-pipeline ~  similar to linked program;
 *  a higher-level construct  cooridnating multiple shader modules; without the details.
 *  works with render pass, material , etc.
 *
 */


class FVkShaderMap
{
public:
	FVkShaderMap(const SharedPtr<FVkDevice> device) : deviceRef(device)
	{
		parameterMap = CreateShared<FVkShaderParamMap>();
	}

	void reflectShaderParameters();
	 
	void allocateDescriptorSets(); 
	void createDescriptorPool();


	void addShaderModule(VkShaderStageFlags stage, SharedPtr<FVkShaderModule> module) {
		shaderModuleMap.insert({ stage, module });
		std::cout << "shader module added" << std::endl;
	}
	std::unordered_map<VkShaderStageFlags, SharedPtr<FVkShaderModule>> shaderModuleMap;


	//own the information can be extracted from SPIR-V reflection; 
	//for the engine
	SharedPtr<FVkShaderParamMap>  parameterMap; //per-pipeline
	std::unordered_map<std::string, FVkShaderParamVertexInputInfo> vertexInputMap;

	std::vector<VkPushConstantRange> pushConstantRanges;  //per-stage

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::array< std::vector<VkDescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorSets; 

	


private:
	const SharedPtr<FVkDevice> deviceRef;

};


class FVkShaderMapBuilder {
public:
	FVkShaderMapBuilder(SharedPtr<FVkDevice>  device) : deviceRef(device) {}

	FVkShaderMapBuilder& SetVertexShader(const std::string& filepath) {
		vertexShader = CreateShared<FVkShaderModule>(deviceRef, VK_SHADER_STAGE_VERTEX_BIT, filepath);
		return *this;
	}

	FVkShaderMapBuilder& SetFragmentShader(const std::string& filepath) {
		fragmentShader = CreateShared<FVkShaderModule>(deviceRef, VK_SHADER_STAGE_FRAGMENT_BIT, filepath);
		return *this;
	}

	SharedPtr<FVkShaderMap> Build() {
		auto shaderMap = CreateShared<FVkShaderMap>(deviceRef);
		if (vertexShader)
			shaderMap->addShaderModule(VK_SHADER_STAGE_VERTEX_BIT, vertexShader);

		if (fragmentShader)
			shaderMap->addShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader);
		//...
		return shaderMap;
	}

private:
	SharedPtr<FVkDevice> deviceRef;
	SharedPtr<FVkShaderModule> vertexShader;
	SharedPtr<FVkShaderModule> fragmentShader;
	//...
};



