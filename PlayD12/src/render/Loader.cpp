#include "PCH.h"

#include "DirectXTex.h"
#include "Loader.h"

namespace Loader {

	std::optional<ImageData> Loader::LoadTexture(std::string_view path, TextureImportConfig config)
	{
        using namespace DirectX;

        ScratchImage image;
        HRESULT hr = LoadFromWICFile(filePath.c_str(), WIC_FLAGS_FORCE_RGBA32, nullptr, image);
        if (FAILED(hr)) {
            std::cerr << "Failed to load font atlas: " << std::string(filePath.begin(), filePath.end()) << std::endl;
            return false;
        }

        const Image* srcImage = image.GetImage(0, 0, 0);
        int texWidth = static_cast<int>(srcImage->width);
        int texHeight = static_cast<int>(srcImage->height);


    }

}