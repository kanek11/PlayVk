#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <optional>


#include <vulkan/vulkan.h>
#include <spirv_reflect.h>
 
#include "GVulkanRHI.h"
#include "Base.h"
 

//std::array<std::vector<VkDescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorSets;

//todo : set max expected resource slots;
//todo:  leverage vertex input info;

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

struct FVkBufferRTInfo
{
	VkBuffer buffer;
	VkDeviceSize offset;
	VkDeviceSize range;
};

struct FVkShaderResBufferInfo
{
	//for layout  
	VkShaderStageFlags   stageFlags{};  
	uint32_t             binding{}; // or BaseIndex; location;   
	uint32_t  	         set{}; 

	//data info
	uint32_t             size{};    // or range; 

	std::optional<FVkBufferRTInfo>  rtInfo{ std::nullopt };
	bool dirty{ false }; 
};

struct FVkTextureRTInfo
{
	VkImage image;
	VkImageView imageView;
	VkSampler sampler;  //storage image doesn't have;
}; 

//image is seen as a immutable whole; so no size or offset;
struct FVkShaderResTextureInfo
{
	VkShaderStageFlags   stageFlags{};
	uint32_t             binding{};
	uint32_t  	         set{};

	std::optional<FVkTextureRTInfo> rtInfo{ std::nullopt };
	bool dirty{ false };
};



//push constant is unique per module, no binding point
struct FVkShaderResPushConstantInfo
{
	VkShaderStageFlags   stageFlags{};
	uint32_t             size{};
};


/*
* static metadata from shader reflection;
* todo: consider more generic?

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
	SamplerMap       samplerMap{};
	UniformBufferMap uniformBufferMap{};
	StorageBufferMap storageBufferMap{};
	PushConstantMap  pushConstantMap{};
	StorageImageMap storageImageMap{};
};

class FVkShaderModule
{
public:
	~FVkShaderModule() {
		if (!deviceRef.expired())
		{
			auto& device = deviceRef.lock()->vkDevice;
			vkDestroyShaderModule(device, shaderModule, nullptr);
           
			this->shaderModule = VK_NULL_HANDLE; 
			std::cout << "Shader module destroyed" << '\n';
		}
		else {
			std::cerr << "destroy shader module after device!" << '\n';
		}
	} 

	explicit FVkShaderModule(WeakPtr<FVkDevice> device, VkShaderStageFlags stage, const std::string& filepath) :
		deviceRef(device), stage(stage)
	{
		readSPIRVFile(filepath);
		createShaderModule();
	}

	FVkShaderModule(const FVkShaderModule&) = delete;
	FVkShaderModule& operator=(const FVkShaderModule&) = delete;

	FVkShaderModule(FVkShaderModule&&) = delete;
	FVkShaderModule& operator=(FVkShaderModule&&) = delete;



	bool createShaderModule();
	void readSPIRVFile(const std::string& filepath); 

public:
	std::vector<uint32_t> SPIRVBytecode;
	VkShaderModule shaderModule { VK_NULL_HANDLE }; 

private:
	const VkShaderStageFlags stage;
	const WeakPtr<FVkDevice> deviceRef;
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
	~FVkShaderMap()
	{
		if (!deviceRef.expired())
		{
			auto& device = deviceRef.lock()->vkDevice; 

			vkDestroyDescriptorPool(device, descriptorPool, nullptr); 
			this->descriptorPool = VK_NULL_HANDLE;

	        std::cout << "Descriptor pool destroyed" << '\n';

			for (auto layout : descriptorSetLayouts) 
			{ 
				std::cout << "Descriptor set layout destroyed:" << layout << '\n';
				vkDestroyDescriptorSetLayout(device, layout, nullptr); 
			}     

			descriptorSetLayouts.clear();

		}
		else {
			std::cerr << "\tapp: destroy descriptors after device!" << '\n';
		} 
	}


	FVkShaderMap(const WeakPtr<FVkDevice> device) : deviceRef(device)
	{
		parameterMap = CreateShared<FVkShaderParamMap>();
	}

	void reflectShaderParameters();

	void createDescriptorPool();
	void allocateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets) const;


	void addShaderModule(VkShaderStageFlags stage, SharedPtr<FVkShaderModule> module) {
		shaderModuleMap.insert({ stage, module });
		std::cout << "shader module added" << '\n';
	}
	std::unordered_map<VkShaderStageFlags, SharedPtr<FVkShaderModule>> shaderModuleMap;


	//own the information can be extracted from SPIR-V reflection; 
	//for the engine
	SharedPtr<FVkShaderParamMap>  parameterMap; //per-pipeline
	std::unordered_map<std::string, FVkShaderParamVertexInputInfo> vertexInputMap; 
	

	//SharedVkHandle<VkDescriptorPool> descriptorPool;
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};


	//in vulkan, layout is assumed contigous, so we can use a vector to store the layout;
	//if the set index is not found, just leave it empty;  
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts; 

	std::vector<VkPushConstantRange> pushConstantRanges;  //per-stage

	//new:  
	// //todo: storage image;
	void bindUniformBuffer(const std::string& name, const FVkBufferRTInfo& bufferInfo);
	void bindStorageBuffer(const std::string& name, const FVkBufferRTInfo& bufferInfo);
	void bindTexture(const std::string& name, const FVkTextureRTInfo& textureInfo);  

	//update on-demand ,works like state machine?
	bool validateBindings() const;
	void updateBindings(std::vector<VkDescriptorSet>& descriptorSets);
	std::vector<FVkShaderResBufferInfo> updateUBOCache;
	std::vector<FVkShaderResBufferInfo> updateSSBOCache;
	std::vector<FVkShaderResTextureInfo> updateTextureCache;


private:
	const WeakPtr<FVkDevice> deviceRef;

};


class FVkShaderMapBuilder {
public:
	FVkShaderMapBuilder(WeakPtr<FVkDevice> device) : deviceRef(device) {}

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
		//todo...
		return shaderMap;
	}

private:
	SharedPtr<FVkShaderModule> vertexShader;
	SharedPtr<FVkShaderModule> fragmentShader;
	//todo... 

	const WeakPtr<FVkDevice> deviceRef;
};



