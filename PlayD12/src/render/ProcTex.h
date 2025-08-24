#pragma once

#include "Base.h"   
#include "Math/MMath.h" 

#include "Texture.h"
#include "Loader.h"

#include "Mesh.h"
#include "ComputePass.h"
 

class FDynamicTex {
public: 
	SharedPtr<FD3D12Texture> texture;   
	virtual void Init() = 0;
	virtual void Dispatch() = 0;  

	Compute::ComputeContext computeCtx;

	//
	SharedPtr<FD3D12Buffer> cbBuffer;
};


class FPlayerTex : public FDynamicTex
{
	virtual void Init();
	virtual void Dispatch();

	struct alignas(256) ParamsCB {
		UINT Width = 1024;
		UINT Height = 1024;
	};

	ParamsCB cbData;

};


