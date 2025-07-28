#pragma once

#include "PCH.h"  

#include "Base.h"
#include "Render/Loader.h"
#include "Render/Texture.h"

inline SharedPtr<FD3D12Texture> LoadTexture(std::string_view path, Loader::TextureImportConfig config = {});


struct AssetManager {
	static AssetManager& Get();

	void LoadResources();

	std::unordered_map<std::string, Loader::D3D12ImageData> textures;
};