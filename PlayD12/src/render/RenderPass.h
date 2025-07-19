#pragma once

#include "PCH.h"     
#include "D12Helper.h"

#include "Shader.h"
#include "Material.h" 
#include "Mesh.h"
#include "Texture.h"

struct RenderPassDesc {
    std::string passTag; // Forward, ShadowCaster, etc.
    DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
    bool enableDepth = true;
    bool enableBlend = false;
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE;
};


struct FRenderPassBindings {
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
    std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;
};

struct AttachmentDesc {
    std::string name;
    SharedPtr<FD3D12Texture> texture;
    ETextureUsage usage; 
};
 
 
struct FRenderPass {
    std::vector<AttachmentDesc> inputs;
    std::vector<AttachmentDesc> outputs;

    void BeginPass(ID3D12GraphicsCommandList* cmd);
    void EndPass(ID3D12GraphicsCommandList* cmd);
	void RecordCommands(ID3D12GraphicsCommandList* cmd);
};








