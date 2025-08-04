#pragma once

#include "PCH.h"

#include "Gameplay/Components/MeshComponent.h"

namespace Gameplay {

	struct FScene { 

		void AddPrimitive(FStaticMeshProxy proxy);

		void SubmitAll();

		std::vector<FStaticMeshProxy> m_sceneProxies; 
	};
	 
}