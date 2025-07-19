#include "PCH.h"
 
#include "Loader.h"
#include <DirectXTex.h>


namespace Loader {
     

    std::optional<D3D12ImageData> LoadTextureDX(std::string_view pathView, TextureImportConfig config)
    {
        using namespace DirectX;

        auto image = CreateShared<ScratchImage>();

        //ScratchImage image;
        std::wstring pathW(pathView.begin(), pathView.end()); 
        HRESULT hr = LoadFromWICFile(pathW.c_str(), WIC_FLAGS_NONE, nullptr, *image);
        if (FAILED(hr)) {
            std::cerr << "Failed to load font atlas: " << pathView << std::endl;
            return std::nullopt;
        }

        const Image* srcImage = image->GetImage(0, 0, 0);

        D3D12ImageMetaInfo metaInfo;
        metaInfo.width = static_cast<int>(srcImage->width);
        metaInfo.height = static_cast<int>(srcImage->height); 
        metaInfo.format = srcImage->format; 
        
        metaInfo.rowPitch = srcImage->rowPitch;
        metaInfo.slicePitch = srcImage->slicePitch;

        D3D12ImageData imageData;
        imageData.data = srcImage->pixels;
        imageData.metaInfo = metaInfo;

        imageData.ownedImage = image;

        return imageData;
    }

}