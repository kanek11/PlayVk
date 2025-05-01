

#include <iostream>  

#include <stb_image.h>

#include "ImageLoader.h"


namespace Utils {

 
}
 

namespace Loader {
	 

	std::optional<ImageData> LoadImage(std::string_view path_view, TextureImportConfig config)
	{
		stbi_set_flip_vertically_on_load(config.bFlipVOnLoad);

		int width, height, channels; 
		int desired_channels = static_cast<int>(config.expectedChannels); 

		const char* path = path_view.data();
		//new:
		if (stbi_info(path, &width, &height, &channels)) {
			if (channels != desired_channels) {
				std::cerr << "Loader warn: Image channels mismatch: " << path << '\n';
				std::cerr << "Actual channels : " << channels << ", Expected channels: " << desired_channels << '\n'; 
			}
		} 

		

		auto loadImageTask = [&]()-> void* {

			if (!config.bIsHDR) {
				stbi_uc* data = stbi_load(path, &width, &height, &channels, desired_channels);
				return data;
			}
			else
			{
				float* data = stbi_loadf(path, &width, &height, &channels, desired_channels);
				return data;
			}

		};

		auto stbiDeleter = [](void* ptr) { stbi_image_free(ptr); }; 
		
		if (void* loadedData = loadImageTask(); loadedData) {

			ImageData imageData{};
			imageData.metaInfo = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels) };
			imageData.data = std::unique_ptr<void, std::function<void(void*)>>(loadedData, stbiDeleter);

			std::cout << "Loader: Loaded image size: " << width << " x " << height << ", channels: " << channels << '\n';
			return imageData;
		}
		else
		{
			std::cerr << "Loader: Failed to load image: " << path << '\n';
			std::cerr << "stbi:" << stbi_failure_reason() << '\n';
			return std::nullopt;
		} 
	}
}

