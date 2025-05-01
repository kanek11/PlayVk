#pragma once

#include <optional> 
#include <memory>
#include <functional>
#include <string_view>
/*
* as agnostic as possible, pure data aspect of image;
*/

namespace Loader {

	struct ImageMetaInfo { 
		uint32_t width{}, height{};
		uint32_t channels{};
	};

 

	struct ImageData {
		ImageMetaInfo metaInfo{};
		std::unique_ptr<void, std::function<void(void*)>> data;
	};

	struct TextureImportConfig
	{
		uint32_t expectedChannels = 4;
		bool bFlipVOnLoad = false;
		bool bIsHDR = false;
	};

	std::optional<ImageData> LoadImage(std::string_view path, TextureImportConfig config = TextureImportConfig{});

}