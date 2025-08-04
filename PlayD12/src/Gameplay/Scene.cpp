#include "PCH.h"
#include "Scene.h"

#include "Application.h"

void Gameplay::FScene::AddPrimitive(FStaticMeshProxy proxy)
{
    m_sceneProxies.push_back(proxy);
}

void Gameplay::FScene::SubmitAll()
{
    //--------------
    auto renderer = GameApplication::GetInstance()->GetRenderer();
    //for (auto& proxy : m_sceneProxies) {
    //    renderer->SubmitMesh(proxy);
    //}
	renderer->SubmitMeshProxies(m_sceneProxies);

	m_sceneProxies.clear();
}
