#pragma once

#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"
#include "Loader.h"

#include "Mesh.h"
#include "PostPass.h"

namespace IBL {
	constexpr UINT CUBE_FACE_SIZE = 2048;
	constexpr UINT IRRADIANCE_SIZE = 256;
	constexpr UINT IRRADIANCE_NUMSAMPLES = 2048;
			   
	constexpr UINT PREFILTER_SIZE = 1024;
	constexpr UINT PREFILTER_NUMSAMPLES = 2048;
			   
	constexpr UINT BRDF_LUT_SIZE = 1024;
	constexpr UINT BRDF_LUT_NUMSAMPLES = 1024;

	constexpr uint32_t MAX_MIP = 5;


	struct alignas(256) SharedCB {
		UINT CUBE_FACE_SIZE = IBL::CUBE_FACE_SIZE;
		UINT IRRADIANCE_SIZE = IBL::IRRADIANCE_SIZE;
		UINT IRRADIANCE_NUMSAMPLES = IBL::IRRADIANCE_NUMSAMPLES;
		UINT PREFILTER_SIZE = IBL::PREFILTER_SIZE;
		UINT PREFILTER_NUMSAMPLES = IBL::PREFILTER_NUMSAMPLES;
		UINT BRDF_LUT_SIZE = IBL::BRDF_LUT_SIZE;
		UINT BRDF_LUT_NUMSAMPLES = IBL::BRDF_LUT_NUMSAMPLES;
	};

}

namespace IBL {



	struct alignas(256) PrefilterCB {
		UINT Resolution; 
		UINT NumSamples; // 1024-2048
		float Roughness;
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

	void Init();
	void Finalize();

	void GenerateBRDFLUT();

	void GenerateIrradiance();
	void GeneratePrefilter();

private:

	Compute::ComputeContext rect2CubeCtx;
	Compute::ComputeContext brdfLUTCtx;
	Compute::ComputeContext irradianceCtx;
	Compute::ComputeContext prefilterCtx;


	//
	SharedPtr<FD3D12Buffer> sharedCB;
	// 
	//per mipmap;
	SharedPtr<FRingBufferAllocator<IBL::PrefilterCB>> prefilterCBAllocator; 
};



