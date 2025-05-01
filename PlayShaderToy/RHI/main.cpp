
#include <iostream>
#include <filesystem>
#include <source_location>



#include "GVulkanRHI.h"

#include "Shader.h"

#include "Buffer.h" 

#include "Texture.h" 
 
#include "Renderer.h"

#include "ComputePipeline.h"
 
 


int main()
{
	auto  src_path = std::source_location::current();
	std::filesystem::path this_file = src_path.file_name();

	std::filesystem::current_path(this_file.parent_path().parent_path());
	std::cout << "Set Current working directory" << std::filesystem::current_path() << std::endl;


	//new: precompile shaders
	std::cout << "\tapp:precompile shaders: " << std::endl;
	if (system("vkCompileAll.bat") != 0)
	{
		throw std::runtime_error("failed to compile shader£¡");
	}
 

 
	auto renderer = CreateShared<FVkGraphicsPipeline>();
	renderer->init();
	renderer->update();

 

	return 0;

}