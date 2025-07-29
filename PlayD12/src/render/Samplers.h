#pragma once

#include "PCH.h"
#include "D12Helper.h"

//D3D12_STATIC_SAMPLER_DESC sampler0 = {};
//sampler0.ShaderRegister = 0;
//sampler0.RegisterSpace = 0;
//sampler0.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
//sampler0.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//sampler0.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//sampler0.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//sampler0.MipLODBias = 0;
//sampler0.MaxAnisotropy = 0;
//sampler0.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
//sampler0.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
//sampler0.MinLOD = 0.0f;
//sampler0.MaxLOD = D3D12_FLOAT32_MAX;
//sampler0.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


namespace Samplers {

    //linear filter + wrap, for common texture;
    static D3D12_STATIC_SAMPLER_DESC LinearWrap(UINT shaderRegister = 0) {
        D3D12_STATIC_SAMPLER_DESC desc = {};
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.ShaderRegister = shaderRegister;
        desc.RegisterSpace = 0;

        return desc;
    }


    static D3D12_STATIC_SAMPLER_DESC LinearClamp(UINT shaderRegister = 0) {
        D3D12_STATIC_SAMPLER_DESC desc = {};
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        desc.MaxLOD = D3D12_FLOAT32_MAX;
        desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.ShaderRegister = shaderRegister;
        desc.RegisterSpace = 0;
        return desc;
    }


    //Pixel perfect;  eg: UI, 
    static D3D12_STATIC_SAMPLER_DESC PointClamp(UINT shaderRegister = 0) {
        D3D12_STATIC_SAMPLER_DESC desc = {};
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        desc.MaxLOD = D3D12_FLOAT32_MAX;
        desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.ShaderRegister = shaderRegister;
        desc.RegisterSpace = 0;

        return desc;
    }

    static D3D12_STATIC_SAMPLER_DESC PointWrap(UINT shaderRegister = 0) {
        D3D12_STATIC_SAMPLER_DESC desc = {};
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        desc.MaxLOD = D3D12_FLOAT32_MAX;
        desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.ShaderRegister = shaderRegister;
        desc.RegisterSpace = 0;

        return desc;
    }


    static D3D12_STATIC_SAMPLER_DESC ShadowMap(UINT shaderRegister = 0) {
        D3D12_STATIC_SAMPLER_DESC desc = {};
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

        desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        desc.ShaderRegister = shaderRegister;
        desc.RegisterSpace = 0;
        return desc;
    }


    static std::unordered_map<std::string, D3D12_STATIC_SAMPLER_DESC> samplerPool =
    {
        { "linearWrapSampler", LinearWrap()},
        { "linearClampSampler", LinearClamp()},
        { "shadowMapSampler", ShadowMap()},
    };
     
}

