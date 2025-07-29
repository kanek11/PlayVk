#include "PCH.h"
 
#include "Loader.h"
#include <DirectXTex.h>


namespace Loader {
     

    std::optional<D3D12ImageData> LoadTextureDX(std::string_view path, TextureImportConfig config)
    {
        std::filesystem::path filePath(path);
        std::wstring ext = filePath.extension().wstring();

        for (auto& c : ext) c = tolower(c);

        if (ext == L".hdr") {
            return LoadHDRTextureDX(path, config);
        }
        else
        {
            return LoadWICTextureDX(path, config);
        }
         
    }

    std::optional<D3D12ImageData> LoadWICTextureDX(std::string_view path, TextureImportConfig config)
    {
        using namespace DirectX;

        DirectX::TexMetadata metaData{};
        ScratchImage image;

        //ScratchImage image;
        std::wstring pathW(path.begin(), path.end());
        HRESULT hr = LoadFromWICFile(pathW.c_str(), WIC_FLAGS_NONE, &metaData, image);
        if (FAILED(hr)) {
            std::cerr << "Loader: Failed to load tex: " << path << std::endl;
            return std::nullopt;
        }

        const Image* srcImage = image.GetImage(0, 0, 0);

        D3D12ImageMetaInfo metaInfo;
        metaInfo.width = static_cast<int>(srcImage->width);
        metaInfo.height = static_cast<int>(srcImage->height);
        metaInfo.format = srcImage->format;

        metaInfo.rowPitch = srcImage->rowPitch;
        metaInfo.slicePitch = srcImage->slicePitch;

        auto ownedPtr = std::make_shared<ScratchImage>(std::move(image));
        D3D12ImageData imageData;
        imageData.metaInfo = metaInfo;
        imageData.data = ownedPtr->GetImage(0, 0, 0)->pixels;  // pointer still valid because ownedPtr is stored
        imageData.ownedImage = std::move(ownedPtr);

        return imageData;
    }

    std::optional<D3D12ImageData> LoadHDRTextureDX(std::string_view pathView, TextureImportConfig config)
    {
        using namespace DirectX;

        DirectX::TexMetadata metaData{};
         
        ScratchImage image;
         
        std::wstring pathW(pathView.begin(), pathView.end());
        HRESULT hr = LoadFromHDRFile(pathW.c_str(), &metaData, image);
        if (FAILED(hr)) {
            std::cerr << "Loader: Failed to load tex: " << pathView << std::endl;
            return std::nullopt;
        }  

        const Image* srcImages = image.GetImages();
        size_t numImages = image.GetImageCount();
        const TexMetadata& info = image.GetMetadata();
        //turns into 
        ScratchImage imageConvert; 
        hr = DirectX::Convert(
            srcImages,
            numImages,
            info,
            DXGI_FORMAT_R16G16B16A16_FLOAT,
            TEX_FILTER_DEFAULT,
            TEX_THRESHOLD_DEFAULT,
            imageConvert
            );

        if (SUCCEEDED(hr)) {
            image = std::move(imageConvert);
            metaData.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        }


        const Image* srcImage = image.GetImage(0, 0, 0);

        D3D12ImageMetaInfo metaInfo;
        metaInfo.width = static_cast<int>(srcImage->width);
        metaInfo.height = static_cast<int>(srcImage->height);
        metaInfo.format = srcImage->format;

        metaInfo.rowPitch = srcImage->rowPitch;
        metaInfo.slicePitch = srcImage->slicePitch;
         
        auto ownedPtr = std::make_shared<ScratchImage>(std::move(image));

        D3D12ImageData imageData;
        imageData.metaInfo = metaInfo;
        imageData.data = ownedPtr->GetImage(0, 0, 0)->pixels;  // pointer still valid because ownedPtr is stored
        imageData.ownedImage = std::move(ownedPtr); 

        return imageData;
    }

}