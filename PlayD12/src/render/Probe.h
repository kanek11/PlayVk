#pragma once
 
#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"
#include "Loader.h"

#include "Mesh.h"
#include "PostPass.h"

class FProbe {
public:
	SharedPtr<FD3D12Texture> envMap; //the raw cubemap
	SharedPtr<FD3D12Texture> diffuseIrradiance;
	SharedPtr<FD3D12Texture> specularPrefilter;
	SharedPtr<FD3D12Texture> brdfLUT;  //2D;

	Float3 position{};
	float radius{}; 
	
	void CreateFromHDRI(SharedPtr<FD3D12Texture>);

private:

	Compute::ComputeContext rect2CubeCtx;
};


namespace IBL {

	struct CubeMapCB {
		UINT cubeSize = 256;
	};
	
}

