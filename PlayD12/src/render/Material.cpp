#include "PCH.h"
#include "Material.h"

#include "Renderer.h"

FMaterialProxy::FMaterialProxy()
{
	auto& graphContext = Render::graphContext;

	textures.insert(
		{ std::string("baseColorMap"), graphContext->fallBackTexture }
	);
}
