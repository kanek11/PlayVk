#pragma once 

#include "PCH.h"
#include <string_view> 
#include <DirectXTex.h>

#include "Base.h"



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
	 
	struct D3D12ImageMetaInfo {
		size_t width{}, height{};
		size_t rowPitch{}, slicePitch{};
		DXGI_FORMAT format{};
	};
	struct D3D12ImageData {
		D3D12ImageMetaInfo metaInfo{};
		uint8_t* data{ nullptr };
		std::shared_ptr<DirectX::ScratchImage> ownedImage;  //ensure the scope of owner;
	};

	std::optional<D3D12ImageData> LoadTextureDX(std::string_view path, TextureImportConfig config = TextureImportConfig{});
	std::optional<D3D12ImageData> LoadWICTextureDX(std::string_view path, TextureImportConfig config = TextureImportConfig{});
	std::optional<D3D12ImageData> LoadHDRTextureDX(std::string_view path, TextureImportConfig config = TextureImportConfig{});
}