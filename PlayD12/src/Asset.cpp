
#include "PCH.h"
#include "Asset.h" 

#include "Render/Renderer.h"

inline SharedPtr<FD3D12Texture> LoadTexture(std::string_view path, Loader::TextureImportConfig config)
{
    auto& ctx = Render::rendererContext;

    if (auto& imgOpt = Loader::LoadTextureDX(path); imgOpt.has_value()) {

        auto metaInfo = imgOpt.value().metaInfo;
        FTextureDesc texDesc =
            FTextureDesc{
            .width = static_cast<UINT>(metaInfo.width),
            .height = static_cast<UINT>(metaInfo.height),
            .format = metaInfo.format,
            .usages = {ETextureUsage::ShaderResource},
        };

        return CreateShared<FD3D12Texture>(ctx->device, texDesc);
    }

    else
    {
        std::cerr << "fail to load tex" << '\n';
        return nullptr;
    }
}

AssetManager& AssetManager::Get()
{
    static AssetManager instance;
    return instance;
}

void AssetManager::LoadResources()
{
    //std::string fileName_ = "assets/textures/rusty_metal_diff.png";
    //if (auto& imgOpt = Loader::LoadTextureDX("assets/textures/rusty_metal_diff.png")) {
    //    this->textures[fileName_] = imgOpt.value();
    //}

    namespace fs = std::filesystem;

    std::string dirPath = "assets/textures/";
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            std::string fullPath = entry.path().string();
            std::string fileName = entry.path().filename().string(); 

            // Load texture using your loader
            if (auto imgOpt = Loader::LoadTextureDX(fullPath)) {
                // Store texture in the map with full path as key (or you can store only filename if you want)
                textures[fileName] = imgOpt.value();
                std::cout << "load image from :" << fullPath << '\n';
            }
            else {
                // Optional: log failure to load texture
                std::cerr << "Failed to load texture: " << fullPath << std::endl;
            }
        }
    }
     

}
