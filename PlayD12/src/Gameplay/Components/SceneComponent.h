#pragma once

#include "PCH.h"
#include "Math/MMath.h"
 
#include "Gameplay/ActorComponent.h"

struct FTransform {
	Float3 position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR rotation{ DirectX::XMQuaternionIdentity() };
	Float3 scale{ 1.0f, 1.0f, 1.0f }; 
};

namespace Gameplay {
	class USceneComponent : public UActorComponent { 
	};


	class UPrimitiveComponent : public USceneComponent {
	};


	class UMeshComponent : public UPrimitiveComponent {
	};

	class UStaticMeshComponent : public UMeshComponent {
	};

}  