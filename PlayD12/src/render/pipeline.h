#pragma once

#include "PCH.h"     
#include "D12Helper.h"

#include "Shader.h"
#include "Material.h" 

#include "RenderPass.h"


struct PSOKey {
    ShaderPermutationKey permutationKey;
    RenderPassDesc pass;

    bool operator==(const PSOKey& other) const {
        return permutationKey == other.permutationKey && pass.passTag == other.pass.passTag;
    }
};

namespace std {
    template<>
    struct hash<PSOKey> {
        size_t operator()(const PSOKey& key) const {
            return hash<ShaderPermutationKey>()(key.permutationKey) ^ hash<std::string>()(key.pass.passTag);
        }
    };
}

class PSOManager {
public:
    PSOManager(ComPtr<ID3D12Device> device, WeakPtr<ShaderLibrary> shaderLibrary) :
        m_device(device), library(shaderLibrary) {
    }

    [[nodiscard]]
    ComPtr<ID3D12PipelineState> GetOrCreate(
        const MaterialDesc& mat,
        const RenderPassDesc& pass,
        const std::vector<VertexInputLayer>& layers);
private:
    ComPtr<ID3D12Device> m_device;
    WeakPtr<ShaderLibrary> library;
    std::unordered_map<PSOKey, ComPtr<ID3D12PipelineState>> cache;
};
