#pragma once

#include "PCH.h" 
#include "Math/MMath.h"

template<typename T, size_t Align = 16>
struct CBMember {
    alignas(Align) T value;
    constexpr CBMember() = default;
    constexpr CBMember(const T& v) : value(v) {}
    operator T& () { return value; }
    operator const T& () const { return value; }
};


struct alignas(256) SceneCB
{
    Float4x4  pvMatrix;
    alignas(16) Float3 cameraPos = Float3(0.0f, 0.0f, 0.0f);

    alignas(16) Float2 viewportSize = Float2(1280.0f, 720.0f);
	float time = 0.0f;
	float deltaTime = 0.0f;
	//uint32_t frameIndex = 0;

    Float4x4  light_pvMatrix;
    alignas(16) Float3 lightDir = Normalize(Float3(0.577f, 0.277f, 0.377f));
    alignas(16) Float3 lightColor = Float3(1.0f, 1.0f, 1.0f); 
	float lightIntensity = 1.0f;

    SceneCB() {
        Float3 lightPos = lightDir * 10.0f;
        Float3 target = Float3(0.0f, 0.0f, 0.0f);

        // 4. Up vector (choose world up, e.g. Y axis)
        Float3 up = Float3(0.0f, 1.0f, 0.0f);

        // 5. Create light view matrix (left-handed)
        Float4x4 lightView = LookAtLH(lightPos, target, up);

        // 6. Create orthographic projection matrix (tweak width/height and near/far planes to cover scene)
        float orthoWidth = 70.0f;
        float orthoHeight = 70.0f;
        float nearZ = 0.1f;
        float farZ = 100.0f;

        Float4x4 lightProj = MMath::OrthographicLH(orthoWidth, orthoHeight, nearZ, farZ);
        //lightProj = Transpose(lightProj);

        // 7. Combine into light-space matrix
        this->light_pvMatrix = MMath::MatrixMultiply(lightProj, lightView);
    }
}; 

//static_assert((sizeof(SceneCB) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

namespace Mesh {



struct alignas(256) ObjectCB
{
    Float4x4 modelMatrix;
	Float4x4 normalMatrix;  
};
 
//static_assert((sizeof(ObjectCB) % 256) == 0, "Constant Buffer size must be 256-byte aligned");



}