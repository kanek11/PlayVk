#pragma once

#include "PCH.h"     
#include "D12Helper.h"

#include "Shader.h"
#include "Material.h" 
#include "Mesh.h"
#include "Texture.h" 

constexpr size_t InvalidTex = UINT32_MAX;


 

struct RenderPassDesc {
    std::string passTag; 
	std::vector<DXGI_FORMAT> colorFormats; 
    DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
    bool enableDepth = true;
    bool enableBlend = false;
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};


// PassRegistry.h
namespace Passes {



}
 

struct FRenderPassAttachments {
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
    std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;
}; 
  

struct RGNode {
    std::string passName;
    std::string bindingName; 

    bool operator==(const RGNode& other) const noexcept {
        return passName == other.passName && bindingName == other.bindingName;
    }

};

namespace std {
    template<>
    struct hash<RGNode> {
        size_t operator()(const RGNode& node) const noexcept {
            size_t h1 = std::hash<std::string>()(node.passName);
            size_t h2 = std::hash<std::string>()(node.bindingName);
            // boost::hash_combine
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}

 
enum class LoadOp { Load, Clear, DontCare };

enum class AccessType { Read, Write };

struct RGTexture { 
	ETextureUsage usage = ETextureUsage::Undefined;
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

	std::optional<RGNode> src; 
    std::optional<FTextureDesc> desc;

	LoadOp loadOp = LoadOp::Clear;
	bool systemOwned = false; 
};

struct RGTextureHandle {
	size_t id{ InvalidTex };
    D3D12_RESOURCE_STATES currState = D3D12_RESOURCE_STATE_COMMON;
	std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> heapHandle; 
};


class RenderGraph;
struct RenderPassBuilder {
    RenderPassBuilder(RenderGraph& graph_, const std::string& name_) : 
		graph(graph_), name(name_) { 
	}
     
    void AddBinding(const std::string& name, const RGTexture& texture); 

    std::string name;
	//uint32_t passId = 0;

    std::unordered_map<std::string, RGTexture> bindingDescs;

    std::function<void(ID3D12GraphicsCommandList*)> executeCB; 

    RenderGraph& graph;
};


 
 
class RenderGraph {
public:
	RenderGraph();
      
    RenderPassBuilder& AddPass(const std::string& name); 
    std::optional<RenderPassBuilder> GetPass(const std::string& name);

    void Compile();
    void Execute(ID3D12GraphicsCommandList* cmdList);

    //expose to passes;
    RGTextureHandle& GetTextureHandle( const RGNode& node);
    SharedPtr<FD3D12Texture> GetTexture(const RGNode& node);
    SharedPtr<FD3D12Texture> GetTexture(const RGTextureHandle& handle) const;

public:
    [[nodiscard]]
    RGTextureHandle& RegisterNewTexture(const RGNode& node, const FTextureDesc& desc);
    RGTextureHandle& RegisterExistingTexture(const RGNode& node, SharedPtr<FD3D12Texture> texture);

private: 
    std::optional<RGTexture> GetBindingInfo(const RGNode& node);

	std::unordered_map<RGNode, RGTextureHandle> nodeBindings;

private:
    std::vector<RenderPassBuilder> passes;

	std::vector<SharedPtr<FD3D12Texture>> textures;
};


