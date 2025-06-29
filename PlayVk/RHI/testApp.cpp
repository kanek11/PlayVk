
#include <iostream>
#include <format>
#include <filesystem>
#include <source_location> 

#include "RHI.h"


bool test = false;


int main()
{
	auto  src_path = std::source_location::current();
	std::filesystem::path this_file = src_path.file_name();

	std::filesystem::current_path(this_file.parent_path().parent_path()); 
	std::cout << "Set Current working directory" << std::filesystem::current_path() << '\n';


	////new: precompile shaders
	//std::cout << "\tapp:precompile shaders: " << '\n';
	//if (system("vkCompileAll.bat") != 0)
	//{
	//	throw std::runtime_error("failed to compile shader£¡");
	//} 

	if (test)
	{
		auto window = CreateShared<FVkWindow>();
		Global::vulkanRHI = new GVulkanRHI(window);

		auto& deviceRef = Global::vulkanRHI->deviceRef;

		auto testModule = CreateShared<FVkShaderModule>(
			deviceRef,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			"shaders/bin/takeInput.glsl.spv"
		);

		auto testShaderMap0 = CreateShared<FVkShaderMap>(deviceRef);
		testShaderMap0->addShaderModule(VK_SHADER_STAGE_VERTEX_BIT, testModule);
		testShaderMap0->reflectShaderParameters();


		auto testShaderMapBuilder = FVkShaderMapBuilder(deviceRef);
		auto testShaderMap = testShaderMapBuilder
			.SetVertexShader("shaders/bin/testVert.glsl.spv")
			.SetFragmentShader("shaders/bin/testFrag.glsl.spv")
			.Build();

		auto shader = testShaderMap->shaderModuleMap.at(VK_SHADER_STAGE_VERTEX_BIT);


		testShaderMap->reflectShaderParameters();


		auto testBuffer = CreateShared<FVkBuffer>(
			deviceRef,
			FVkBufferDesc { eUNIFORM_BUFFER, 1024 } 
		);

		struct UBO {
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 proj = glm::mat4(1.0f);
		};
		UBO ubo = {};

		auto& heapManager = Global::vulkanRHI->heapManagerRef;
		heapManager->updateBufferMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, testBuffer->buffer, testBuffer->memory, sizeof(UBO), &ubo);
		 


		auto testMeshObj = CreateShared<UStaticMesh>();

		std::vector<glm::vec3> pos{
			glm::vec3 {0.0f, -0.5f, 0.0f},
			glm::vec3 {0.5f, 0.5f, 0.0f},
			glm::vec3 {-0.5f, 0.5f, 0.0f}, 
		};

		std::vector<uint32_t> indices{
			0, 1, 2
		};

		testMeshObj->positions = pos;
		testMeshObj->indices = indices;

		testMeshObj->CreateRHIResource(); 
 

		delete Global::vulkanRHI;
	}

	if(!test)
	{ 
		try {
			auto renderer = CreateShared<FVkRenderer>();
			renderer->OnInit();
			renderer->OnUpdate();
			renderer->OnShutDown();
		}
		catch (const std::runtime_error& e) {
			std::cerr << "captured error:" << e.what() << '\n';
		}
		
	}
	  

	//hold:
    std::cin.get();  

}