#pragma once


template <typename VkHandle_T>
using SharedVkHandle = std::shared_ptr<std::decay_t<VkHandle_T>>;

template <typename VkHandle_T>
SharedVkHandle<VkHandle_T> CreateSharedVkhandle(
    VkHandle_T handle, std::function<void(VkHandle_T)> dtor) {
  return std::make_shared<std::decay_t<VkHandle_T>>(handle, dtor);
}


template <typename VkHandle_T, typename Deleter_T>
class VulkanResource {
 public:
  VulkanResource(VkHandle_T handle, VkDevice device, Deleter_T deleter)
      : handle_(handle), device_(device), deleter_(deleter) {}

  ~VulkanResource() {
    if (handle_ != VK_NULL_HANDLE) {
      deleter_(device_, handle_);
    }
  }

  // Disable copying to prevent multiple owners
  VulkanResource(const VulkanResource&) = delete;
  VulkanResource& operator=(const VulkanResource&) = delete;

  // Enable move semantics
  VulkanResource(VulkanResource&& other) noexcept
      : handle_(other.handle_),
        device_(other.device_),
        deleter_(other.deleter_) {
    other.handle_ = VK_NULL_HANDLE;
  }

  VulkanResource& operator=(VulkanResource&& other) noexcept {
    if (this != &other) {
      handle_ = other.handle_;
      device_ = other.device_;
      deleter_ = other.deleter_;
      other.handle_ = VK_NULL_HANDLE;
    }
    return *this;
  }

  VkHandle_T get() const { return handle_; }

 private:
  VkHandle_T handle_;
  VkDevice device_;
  Deleter_T deleter_;
};

// Helper function to create a shared Vulkan resource
template <typename VkHandle_T, typename Deleter_T>
std::shared_ptr<VulkanResource<VkHandle_T, Deleter_T>> CreateSharedVkHandle(
    VkHandle_T handle, VkDevice device, Deleter_T deleter) {
  return std::make_shared<VulkanResource<VkHandle_T, Deleter_T>>(handle, device,
                                                                 deleter);
}




// experimental
template <typename VkHandle_T>
class UniqueVkObject {
 public:
  UniqueVkObject() {
    handle = VK_NULL_HANDLE;
    dtor = [](VkHandle_T handle) {};
  }
  explicit UniqueVkObject(VkHandle_T _handle,
                          std::function<void(VkHandle_T)> _dtor)
      : handle(_handle), dtor(_dtor) {}

  // Move constructor
  UniqueVkObject(UniqueVkObject&& other) noexcept
      : handle(other.handle), dtor(std::move(other.dtor)) {
    other.handle = VK_NULL_HANDLE;
  }

  // Move assignment operator
  UniqueVkObject& operator=(UniqueVkObject&& other) noexcept {
    if (this != &other) {
      reset();  // Destroy current handle
      handle = other.handle;
      dtor = std::move(other.dtor);
      other.handle = VK_NULL_HANDLE;
    }
    return *this;
  }

  // Destructor
  ~UniqueVkObject() {
    reset();  // Destroy the handle if valid
  }

  void reset() {
    if (handle != VK_NULL_HANDLE) {
      dtor(handle);
      handle = VK_NULL_HANDLE;
    }
    std::cout << "destructed???" << '\n';
  }
  VkHandle_T get() const { return handle; }

 private:
  VkHandle_T handle;
  std::function<void(VkHandle_T)> dtor;
};

template <typename VkHandle_T>
UniqueVkObject<VkHandle_T> CreateUniqueVkObject(
    VkHandle_T handle, std::function<void(VkHandle_T)> dtor) {
  return UniqueVkObject<VkHandle_T>(handle, dtor);





  layout(set = 0, binding = 0, std430) readonly buffer ParticleIn_t {
	Particle particlesIn[];
} ParticleIn;


std::vector< VkSemaphore> waitSemaphores = {
		imageAvailableSemaphores[currentFrame] ,
		computeFinishedSemaphores[currentFrame]
};

//new: 
computePipeline->UpdateInputs(SystemInputUBO{ static_cast<float>(deltaTime) });
//std::cout << "deltatime" << deltaTime << '\n';
computePipeline->Compute(computeFinishedSemaphores);



FVkBufferRTInfo rtInfo = { computePipeline->particleSSBOs[iFrame]->buffer, 0,
	static_cast<VkDeviceSize>(sizeof(ParticleSSBO) * PARTICLE_COUNT) };
shaderMap->bindStorageBuffer("ParticleIn", rtInfo);

 
//new:  
//a test threading:
this->computePipeline = CreateShared<FVkComputePipeline>();
std::thread computeInitThread(&FVkComputePipeline::init, computePipeline);
computeInitThread.join();



//new:
FVkTextureRTInfo textureInfo = { proxy.texture.lock()->image,  proxy.texture.lock()->imageView, proxy.texture.lock()->sampler };
shaderMap->bindTexture("texSampler", textureInfo);
shaderMap->updateBindings(currentFrame);


if (sceneOpt->meshes.size() > 0) {

	size_t totalVertices = 0;
	size_t totalIndices = 0;

	for (const auto& subMesh : sceneOpt->meshes) {
		totalVertices += subMesh.positions.size();
	}
	cout << "total vertices: " << totalVertices << '\n';

	auto mesh = CreateShared<UStaticMesh>();

	mesh->positions.reserve(totalVertices);
	mesh->UVs.reserve(totalVertices);
	mesh->indices.reserve(totalIndices);

	for (const auto& subMesh : sceneOpt->meshes)
	{
		mesh->positions.insert(mesh->positions.end(), subMesh.positions.begin(), subMesh.positions.end());
		mesh->UVs.insert(mesh->UVs.end(), subMesh.UVs.begin(), subMesh.UVs.end());

		uint32_t offset = mesh->indices.size();
		for (auto index : subMesh.indices) {
			mesh->indices.push_back(index + offset);
		}

	}

	cout << "loaded vertices: " << mesh->positions.size() << '\n';
	cout << "loaded UVs: " << mesh->UVs.size() << '\n';
	cout << "loaded indices: " << mesh->indices.size() << '\n';

	//
	mesh->CreateRHIResource();

	//
	FStaticMeshObjProxy proxy{
	 .vertexBuffer = mesh->staticMeshResource->vertexBuffer->buffer,
	 .indexBuffer = mesh->staticMeshResource->indexBuffer->buffer,
	 .indexCount = static_cast<uint32_t>(mesh->indices.size()),
	 .indexType = VK_INDEX_TYPE_UINT32,
	 .textureIndex = 0, //for now:
	};

	meshes.push_back(mesh);
	meshProxies.push_back(proxy);
}




//define a rectangle
std::vector<glm::vec3> pos = {
	glm::vec3 {-0.5f, -0.5f, 0.0f },
	glm::vec3 { 0.5f, -0.5f, 0.0f },
	glm::vec3 { 0.5f,  0.5f, 0.0f },
	glm::vec3 {-0.5f,  0.5f, 0.0f },

	glm::vec3 {-0.5f, -0.5f, 0.0f } + glm::vec3(0.2f,0.2f,0.1f),
	glm::vec3 { 0.5f, -0.5f, 0.0f } + glm::vec3(0.2f,0.2f,0.1f),
	glm::vec3 { 0.5f,  0.5f, 0.0f } + glm::vec3(0.2f,0.2f,0.1f),
	glm::vec3 {-0.5f,  0.5f, 0.0f } + glm::vec3(0.2f,0.2f,0.1f),
};

std::vector<uint32_t> indices = {
	 0, 1, 2, 2, 3, 0,
	 4, 5, 6, 6, 7, 4
};

std::vector<glm::vec2> UV =
{
   glm::vec2 { 1.0f, 0.0f },
   glm::vec2 { 0.0f, 0.0f },
   glm::vec2 { 0.0f, 1.0f },
   glm::vec2 { 1.0f, 1.0f },

	glm::vec2 { 1.0f, 0.0f },
	glm::vec2 { 0.0f, 0.0f },
	glm::vec2 { 0.0f, 1.0f },
	glm::vec2 { 1.0f, 1.0f },




};
 

mesh->positions = pos;
mesh->UVs = UV;
mesh->indices = indices;




std::cout << "reflect: bind ssbo :" << name << '\n';
std::cout << "reflect: bound to: set: " << bindInfo.set << ", binding: " << bindInfo.binding << '\n';


std::cout << "Debug - bufferInfoCache contents:\n";
for (const auto& bufferInfo : bufferInfoCache) {
	std::cout << "  buffer = " << bufferInfo.buffer
		<< ", offset = " << bufferInfo.offset
		<< ", range = " << bufferInfo.range << '\n';
}

for (const auto& imageInfo : imageInfoCache) {
	std::cout << "  imageView = " << imageInfo.imageView
		<< ", sampler = " << imageInfo.sampler << '\n';
}

std::cout << "update descriptor num: " << bindingCache.size() << '\n';

for (const auto& write : bindingCache) {
	if (write.pBufferInfo) {
		if (write.pBufferInfo->buffer == VK_NULL_HANDLE) {
			std::cerr << "Invalid VkBuffer handle detected in descriptor update.\n";
		}
		if (write.pBufferInfo->offset != 0) {
			std::cerr << "possibly corrupt data.\n";

		}
	}
	if (write.pImageInfo) {
		if (write.pImageInfo->imageView == VK_NULL_HANDLE || write.pImageInfo->sampler == VK_NULL_HANDLE) {
			std::cerr << "Invalid VkImageView or VkSampler handle detected in descriptor update.\n";
		}
	}
}




void FVkShaderMap::updateDescriptorSets()
{
	auto device = deviceRef.lock()->vkDevice;


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		
		//cache that ensures the binding info is not out of scope
		std::vector<VkWriteDescriptorSet> descriptorWrites;
		std::vector <VkDescriptorBufferInfo> bufferInfoCache;
		std::vector <VkDescriptorImageInfo> imageInfoCache;


		auto& storageBufferMap = parameterMap->storageBufferMap;
		auto& uniformBufferMap = parameterMap->uniformBufferMap;
		auto& samplerMap = parameterMap->samplerMap;


		for (auto& [name, bindInfo] : uniformBufferMap)
		{
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				continue;
			}
			//VkDescriptorBufferInfo bufferInfo{};
			bufferInfoCache.emplace_back();
			VkDescriptorBufferInfo& bufferInfo = bufferInfoCache.back();
			bufferInfo.buffer = bindInfo.rtInfo->buffer;
			bufferInfo.offset = bindInfo.rtInfo->offset;
			bufferInfo.range = bindInfo.rtInfo->range;

			VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			descriptorWrite.dstSet = descriptorSets[i][bindInfo.set];
			descriptorWrite.dstBinding = bindInfo.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			descriptorWrites.push_back(descriptorWrite);
		}


		for (auto& [name, bindInfo] : storageBufferMap)
		{
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				continue;
			}

			bufferInfoCache.emplace_back();
			VkDescriptorBufferInfo& bufferInfo = bufferInfoCache.back();
			bufferInfo.buffer = bindInfo.rtInfo->buffer;
			bufferInfo.offset = bindInfo.rtInfo->offset;
			bufferInfo.range = bindInfo.rtInfo->range;

			VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			descriptorWrite.dstSet = descriptorSets[i][bindInfo.set];
			descriptorWrite.dstBinding = bindInfo.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			descriptorWrites.push_back(descriptorWrite);
		}


		for (auto& [name, bindInfo] : samplerMap)
		{
			if (!bindInfo.rtInfo.has_value())
			{
				std::cerr << "Shader: resource not bound: " << name << '\n';
				continue;
			}

			imageInfoCache.emplace_back();
			VkDescriptorImageInfo& imageInfo = imageInfoCache.back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = bindInfo.rtInfo->imageView;
			imageInfo.sampler = bindInfo.rtInfo->sampler;

			VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			descriptorWrite.dstSet = descriptorSets[i][bindInfo.set];
			descriptorWrite.dstBinding = bindInfo.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			descriptorWrites.push_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}

}





// Prepare descriptor write
VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
descriptorWrite.dstSet = descriptorSets[iFrame][bindInfo.set];
descriptorWrite.dstBinding = bindInfo.binding;
descriptorWrite.dstArrayElement = 0;
descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;  //to: fold only difference
descriptorWrite.descriptorCount = 1;
descriptorWrite.pBufferInfo = &bufferInfo;

// Update cache
this->bindingInfoCache.push_back(descriptorWrite);

void  FVkRenderer::updateDescriptorSets()
{
	auto device = deviceRef.lock()->vkDevice;
	auto textureImageView = texture->imageView;
	auto textureSampler = texture->sampler;

	auto imageBindings = shaderMap->parameterMap->samplerMap;
	const char* name = "texSampler";
	auto imageBinding = imageBindings.at(name);
	std::cout << "reflect: " << name << " bound to " << imageBinding.binding << '\n';

	auto ssaoBindings = shaderMap->parameterMap->storageBufferMap;
	auto ssaboBinding = ssaoBindings.at("ParticleIn");
	std::cout << "reflect: " << "ParticleIn" << " bound to " << ssaboBinding.binding << '\n';


	auto descriptorSets = shaderMap->descriptorSets;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		auto descriptorSets0 = descriptorSets[i][0];
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = computePipeline->particleSSBOs[i]->buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ParticleSSBO) * PARTICLE_COUNT;

		VkWriteDescriptorSet ssaoWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		ssaoWrite.dstSet = descriptorSets0;
		ssaoWrite.dstBinding = ssaboBinding.binding;
		ssaoWrite.dstArrayElement = 0;
		ssaoWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssaoWrite.descriptorCount = 1;
		ssaoWrite.pBufferInfo = &bufferInfo;

		descriptorWrites.push_back(ssaoWrite);


		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		VkWriteDescriptorSet samplerWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		samplerWrite.dstSet = descriptorSets0;
		samplerWrite.dstBinding = imageBinding.binding;
		samplerWrite.dstArrayElement = 0;
		samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerWrite.descriptorCount = 1;
		samplerWrite.pImageInfo = &imageInfo;

		descriptorWrites.push_back(samplerWrite);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}

}
