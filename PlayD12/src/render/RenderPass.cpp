#include "PCH.h"
#include "RenderPass.h"

#include "D12Helper.h" 

#include "Renderer.h"

RenderGraph::RenderGraph()
{
	passes.reserve(100); // reserve some space for passes
}

RenderPassBuilder& RenderGraph::AddPass(const std::string& name)
{
	passes.emplace_back(RenderPassBuilder(*this, name));

	return passes.back();
}
 

void RenderGraph::Compile()
{
	auto& renderCtx = Render::rendererContext;

	//lambda that create rtv/dsv: 
	auto createAttachmentViews = [&](SharedPtr<FD3D12Texture> tex, ETextureUsage usage)
		-> std::optional<D3D12_CPU_DESCRIPTOR_HANDLE>
		{
			switch (usage) {
			case ETextureUsage::RenderTarget:
			{
				auto rtvDesc = tex->GetRTVDesc();
				auto offset = renderCtx->rtvHeapAllocator->Allocate(1);
				auto rtvHandle = renderCtx->rtvHeapAllocator->GetCPUHandle(offset);
				renderCtx->device->CreateRenderTargetView(tex->GetRawResource(), &rtvDesc, rtvHandle);

				return rtvHandle;
				break;
			}
			case ETextureUsage::DepthStencil:
			{
				auto dsvDesc = tex->GetDSVDesc();
				auto offset = renderCtx->dsvHeapAllocator->Allocate(1);
				auto dsvHandle = renderCtx->dsvHeapAllocator->GetCPUHandle(offset);
				renderCtx->device->CreateDepthStencilView(tex->GetRawResource(), &dsvDesc, dsvHandle);

				return dsvHandle;
				break;
			}
			default:
				return std::nullopt;
				break;
			}
		};


	for (auto& pass : passes) {
		for (const auto& [texName, bindingInfo] : pass.bindingDescs) {

			if (bindingInfo.systemOwned) continue;

			if (!bindingInfo.src.has_value() && bindingInfo.desc.has_value()) {

				auto& texHandle = this->RegisterNewTexture(RGNode{ pass.name, texName }, bindingInfo.desc.value());
				auto& tex = this->GetTexture(texHandle);
				texHandle.currState = bindingInfo.state;
				texHandle.heapHandle = createAttachmentViews(tex, bindingInfo.usage);

			}
			else if (!bindingInfo.src.has_value() && !bindingInfo.desc.has_value()) {
				std::cerr << "Error: Texture binding '" << pass.name << "' must have src or desc.\n";
			}
			else
			{
			}

		}
	}






}

void RenderGraph::Execute(ID3D12GraphicsCommandList* cmdList)
{
	auto& renderCtx = Render::rendererContext;

	for (auto& pass : passes)
	{
		if (!pass.executeCB)  return;

		//bind rtv/dsv if needed 
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
		std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;

		std::optional<uint32_t> bufferWidth{};
		std::optional<uint32_t> bufferHeight{};

		//transition the texture state if needed
		for (const auto& [texName, bindingInfo] : pass.bindingDescs) {

			//no transition if the texture is not shared by passes;
			//if (!bindingInfo.src.has_value()) continue;

			auto thisNode = RGNode{ pass.name, texName };
			auto& texHandle = GetTextureHandle(thisNode);
			auto& texture = GetTexture(texHandle);
			D3D12_RESOURCE_STATES targetState = bindingInfo.state;

			if (texHandle.currState != targetState) {
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					texture->GetRawResource(),
					texHandle.currState,
					targetState
				);
				cmdList->ResourceBarrier(1, &barrier);
				texHandle.currState = targetState;

				std::cout << "Transit texture state:" << texName << '\n';
			}


			//clear the texture if needed 
			if (!texHandle.heapHandle.has_value()) continue;

			auto heapHandle = texHandle.heapHandle.value();
			bufferWidth = texture->GetDesc().width;
			bufferHeight = texture->GetDesc().height;

			if (bindingInfo.usage == ETextureUsage::RenderTarget) {
				rtvs.push_back(heapHandle);

				if (bindingInfo.loadOp == LoadOp::Clear) {
					auto clearColor = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
					cmdList->ClearRenderTargetView(heapHandle, clearColor.data(), 0, nullptr);
				}
			}
			else if (bindingInfo.usage == ETextureUsage::DepthStencil) {
				dsv = heapHandle;

				if (bindingInfo.loadOp == LoadOp::Clear) {
					cmdList->ClearDepthStencilView(heapHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
				}
			}


		}

		if (!rtvs.empty() || dsv.has_value()) {
			auto dsvHandlePtr = dsv.has_value() ? &dsv.value() : nullptr;
			cmdList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(),
				FALSE,
				dsvHandlePtr);

			std::cout << "Render pass '" << pass.name << "' has "
				<< rtvs.size() << " RTVs and "
				<< (dsv.has_value() ? "1 DSV." : "0 DSV.") << '\n';

		}

		if (bufferWidth.has_value() && bufferHeight.has_value()) {
			auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(bufferWidth.value()), static_cast<float>(bufferHeight.value()));
			auto scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(bufferWidth.value()), static_cast<LONG>(bufferHeight.value()));
			cmdList->RSSetViewports(1, &viewport);
			cmdList->RSSetScissorRects(1, &scissorRect);
		}
		else
		{
			std::cerr << "Error: Render pass '" << pass.name << "' has no known attachments.\n";
		}

		// Execute the pass's command list

		pass.executeCB(cmdList);
	}
}

RGTextureHandle& RenderGraph::GetTextureHandle(
	const RGNode& thisNode)
{
	auto currentNode = thisNode;
	auto bindingOpt = GetBindingInfo(currentNode);
	if (!bindingOpt.has_value()) {
		std::cerr << "Error: didn't find tex for node: " << currentNode.passName << " - " << currentNode.bindingName << '\n';
		return nodeBindings[currentNode];
	}

	auto currentBinding = bindingOpt.value();

	while (!nodeBindings.contains(currentNode))
	{
		if (!currentBinding.src.has_value()) {
			std::cerr << "Error: didn't find tex for node: " << currentNode.passName << " - " << currentNode.bindingName << '\n';
			return nodeBindings[currentNode];
		}

		currentNode = currentBinding.src.value();
	}

	return nodeBindings.at(currentNode);

}

SharedPtr<FD3D12Texture> RenderGraph::GetTexture(const RGNode& node)
{
	auto handle = GetTextureHandle(node);
	return GetTexture(handle);
}

SharedPtr<FD3D12Texture> RenderGraph::GetTexture(const RGTextureHandle& handle) const
{
	assert(handle.id < textures.size());
	return textures[handle.id];
}

RGTextureHandle& RenderGraph::RegisterNewTexture(const RGNode& node, const FTextureDesc& desc)
{
	auto& renderCtx = Render::rendererContext;

	RGTextureHandle handle{ .id = textures.size() };

	auto texture = CreateShared<FD3D12Texture>(renderCtx->device, desc);

	textures.push_back(texture);
	nodeBindings[node] = handle;


	std::cout << "Create texture for: " << node.passName << " - " << node.bindingName << '\n';
	return nodeBindings[node];

}

RGTextureHandle& RenderGraph::RegisterExistingTexture(const RGNode& node, SharedPtr<FD3D12Texture> texture)
{
	RGTextureHandle handle{ .id = textures.size() };
	textures.push_back(texture);
	nodeBindings[node] = handle;
	std::cout << "Register existing texture for: " << node.passName << " - " << node.bindingName << '\n';
	return nodeBindings[node];

}

std::optional<RenderPassBuilder> RenderGraph::GetPass(const std::string& name)
{
	for (auto& pass : passes) {
		if (pass.name == name) {
			return pass;
		}
	}
	std::cerr << "Error: Render pass '" << name << "' not found.\n";
	return std::nullopt;
}


std::optional<RGTexture> RenderGraph::GetBindingInfo(const RGNode& node)
{
	if (auto passOpt = GetPass(node.passName)) {
		if (passOpt.value().bindingDescs.contains(node.bindingName)) {
			return passOpt.value().bindingDescs.at(node.bindingName);
		}
	}

	std::cerr << "Error: didn't find node: " << node.passName << " - " << node.bindingName << '\n';
	return std::nullopt;

}

void RenderPassBuilder::AddBinding(const std::string& name, const RGTexture& texture)
{
	bindingDescs[name] = texture;
}