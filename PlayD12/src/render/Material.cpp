#include "PCH.h"
#include "Material.h"

#include "Renderer.h"



UMaterial::UMaterial()
{
	auto& graphContext = Render::graphContext;

	textures.insert( { "baseColorMap", "Checkerboard" } );
}