#pragma once

#include "PCH.h"

#include "Render/StaticMeshProxy.h"

#include "Render/SceneViewProxy.h"

namespace Gameplay {
	 
	struct FScene {
		 
		void SubmitAll(); 

		void AddPrimitive(const FStaticMeshProxy& proxy);
		std::vector<FStaticMeshProxy> m_sceneProxies;

		void AddSceneView(const FSceneView& sceneView);
		FSceneView sceneView;
	};

}