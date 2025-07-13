#pragma once

#include "PCH.h"	

#include "Base.h"
#include "Window.h"
#include "Render/Renderer.h"
#include "Physics/PhysicsScene.h"

#include "InputSystem.h"
#include "FSM.h"
#include "Level.h"

#include "UI.h"

//the frontend of an application is composed of interactive elements.
//window is usually considered as the top level UI element.  as a container.
//layer is a way to group content, visually and logically. 
//usually assume layers live within a window.
//we only assume a top level window for now.
//extend as needed.

//todo:
//event handler.
//a layer capture and handle window events.  block as needed.



class LayerBase {
public:
	LayerBase() = default;
	virtual ~LayerBase() = default;

	virtual void onInit() = 0; 
	virtual void onUpdate() = 0;
	virtual void onDestroy() = 0;
	 
};

//background for rendering game scene.
class BaseLayer : public LayerBase {
public:
	BaseLayer() = default;
	virtual ~BaseLayer() = default;
	virtual void onInit() override;
	virtual void onUpdate() override;
	virtual void onDestroy() override;
};


class GUILayer : public LayerBase {
public:
	GUILayer() = default;
	virtual ~GUILayer() = default;
	virtual void onInit() override;
	virtual void onUpdate() override;
	virtual void onDestroy() override;
};


class ApplicationBase {
public:
	ApplicationBase() = default;
	virtual ~ApplicationBase() = default; 
	virtual void onInit() = 0; 
	virtual void onDestroy() = 0; 
	virtual void run() = 0;  
};



class GameApplication : public ApplicationBase {
public:
	GameApplication();
	virtual ~GameApplication() = default;
	virtual void onInit() override;
	virtual void onDestroy() override;
	virtual void run() override; 


	//
	float getAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	PhysicsScene* GetPhysicalScene() { return m_physicsScene; }
	D3D12HelloRenderer* GetRenderer() { return  m_renderer; }
	InputSystem* GetInputSystem() { return  m_inputSystem; }
	GameStateManager* GetGameStateManager() { return m_gameManager; }
	UIManager* GetUIManager() { return m_uiManager; }

private:
	[[nodiscard]] bool initWorkingDirectory();
	[[nodiscard]] bool initWindow();

protected:
	//std::vector<SharedPtr<LayerBase>> m_layers;  //add,remove,clear..
	//hardocde for simplicity.  todo:
	SharedPtr<BaseLayer> m_baseLayer;
	SharedPtr<GUILayer> m_guiLayer;

private:
	static GameApplication* s_instance; 
public:
	static GameApplication* GetInstance();

protected:
	SharedPtr<WindowBase> m_mainWindow;

	uint32_t m_width = 1280;
	uint32_t m_height = 720;
	std::string m_title = "Default Application";

private:
	std::string m_workingDirectory;

public:
	std::string GetAssetFullPath(std::string assetName);

private:
	D3D12HelloRenderer* m_renderer{ nullptr };
	PhysicsScene* m_physicsScene{ nullptr };
	InputSystem* m_inputSystem{ nullptr };
	GameStateManager* m_gameManager{ nullptr };

	UIManager* m_uiManager{ nullptr };

private:
	//hardcode for now:
	GamePlayWorld* gameWorld{ nullptr };
	MainMenuWorld* mainMenuWorld{ nullptr };
};