#pragma once
 
#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"
#include "Loader.h"

#include "Mesh.h"
#include "PostPass.h"

namespace IBLConfig {
	constexpr int CUBE_SIZE = 256;  
	constexpr int IRRADIANCE_SIZE = 32; 
	constexpr int PREFILTER_SIZE = 128; 
	constexpr int BRDF_LUT_SIZE = 1024;
}

namespace IBL {

	struct CubeMapCB {
		UINT cubeSize = 256;
	};

}

class FProbe {
public:
	SharedPtr<FD3D12Texture> envMap; //the raw cubemap
	SharedPtr<FD3D12Texture> diffuseIrradiance;  //cubemap
	SharedPtr<FD3D12Texture> specularPrefilter;  //cubemap with miplevels
	SharedPtr<FD3D12Texture> brdfLUT;  //2D;

	Float3 position{};
	float radius{}; 
	
	void CreateFromHDRI(SharedPtr<FD3D12Texture>);

	void GenerateBRDFLUT();

	void GenerateIrradiance();
	void GeneratePrefilter();

private:

	Compute::ComputeContext rect2CubeCtx; 
	Compute::ComputeContext brdfLUTCtx;  
	Compute::ComputeContext irradianceCtx;
	Compute::ComputeContext prefilterCtx;
};




