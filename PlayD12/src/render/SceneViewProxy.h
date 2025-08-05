#pragma once
 
#include "PCH.h"  
#include "Math/MMath.h"    

struct FSceneView {
    Float4x4 pvMatrix;
    Float4x4 invProjMatrix;
    Float4x4 invViewMatrix;
    Float3 position; 
}; 

