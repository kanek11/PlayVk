#include "PCH.h"
#include "Application.h"

GameApplication* GameApplication::s_instance = nullptr;


GameApplication::GameApplication()
{
	s_instance = this;
}

void GameApplication::onInit()
{
	//---------------------------- 
	if (!initWorkingDirectory())
	{
		throw std::runtime_error("Failed to initialize working directory");
	}

	std::cout << this->GetAssetFullPath("Shaders/shaders.hlsl") << "\n";

	//----------------------------
	if (!initWindow()) {

		throw std::runtime_error("Failed to initialize window");

	}

	auto wideTitle = std::wstring(m_title.begin(), m_title.end());


	//----------------------------
	m_physicsScene = new PhysicsScene();
	m_physicsScene->OnInit();

	//----------------------------
	m_renderer = new D3D12HelloRenderer(m_width, m_height, wideTitle.c_str(), m_mainWindow);
	m_renderer->OnInit();


	//----------------------------
	m_inputSystem = new InputSystem();

	//m_mainWindow->InitInputSource(m_inputSystem); 


	//-----------------------------
	m_uiManager = new UIManager(); 


	//----------------------------
	m_gameManager = new GameStateManager();

	//---------------------- 
	m_worldManager = new WorldManager();
	 
}

void GameApplication::onDestroy()
{
	m_renderer->OnDestroy();
}

void GameApplication::run()
{
	//todo:  exit condition could be more complex.
	while (!m_mainWindow->shouldClose()) {

		//std::cout << "tick app" << '\n';
		m_mainWindow->onUpdate(); 

		m_physicsScene->Tick(0.016f);

		m_renderer->OnUpdate(0.016f);
		m_renderer->OnRender();


		m_inputSystem->OnUpdate();

		m_uiManager->ProcessEvents();
		//if (m_inputSystem->IsKeyJustPressed(KeyCode::A)) {
		//	std::cout << "app: A is pressed" << '\n';
		//}
		//else if (m_inputSystem->IsKeyJustReleased(KeyCode::A)) {
		//	std::cout << "app: A is released" << '\n';
		//}
		//else if (m_inputSystem->IsKeyDown(KeyCode::A)) {
		//	//std::cout << "app: A is down" << '\n';
		//}
		//else if (m_inputSystem->IsKeyUp(KeyCode::A)) {
		//	//std::cout << "app: A is up" << '\n';
		//}

		if (m_inputSystem->IsKeyJustPressed(KeyCode::Num1)) {
			std::cout << "app: 1 is pressed" << '\n';
			m_gameManager->TransitState(GameStateId::Playing);
		}
		else
			if (m_inputSystem->IsKeyJustPressed(KeyCode::Num2)) {
				std::cout << "app: 2 is pressed" << '\n';
				m_gameManager->TransitState(GameStateId::MainMenu);
			}

		m_gameManager->Update(0.016f);


		m_worldManager->Update(0.016f);

	}


}

void GameApplication::onBeginGame()
{

	m_worldManager->RegisterWorld("gameplay", CreateShared<GamePlayWorld>());
	m_worldManager->RegisterWorld("mainMenu", CreateShared<MainMenuWorld>()); 


	if (auto mainMenuState = m_gameManager->Register<MainMenuState>()) {
		mainMenuState->OnEnter.Add(
			[&]() {
				std::cout << " enter mainMenu state" << "\n";
				m_worldManager->TransitWorld("mainMenu");
				//mainMenuWorld->OnLoad();
			});

		mainMenuState->OnUpdate.Add(
			[&](float dt) {
				//mainMenuWorld->OnUpdate(dt);
				//std::cout << " tick mainMenu: " << dt << "\n";
			});
		mainMenuState->OnExit.Add(
			[&]() {
				//mainMenuWorld->OnUnload();
				//std::cout << " tick mainMenu: " << dt << "\n";
			});

		//m_gameManager->SetInitialState(mainMenuState->GetId());
	}



	if (auto playingState = m_gameManager->Register<PlayingState>()) {
		 
		playingState->OnEnter.Add(
			[&]() {
				std::cout << " enter playing state" << "\n";
				//gameWorld->OnLoad();

				m_worldManager->TransitWorld("gameplay");
			});
		
		playingState->OnUpdate.Add(
			[&](float dt) {
				//gameWorld->OnUpdate(dt);
				//std::cout << " tick playing: " << dt << "\n";
			});
		playingState->OnExit.Add(
			[&]() {
				std::cout << " exit playing state" << "\n";
				//gameWorld->OnUnload();
			});

		m_gameManager->SetInitialState(playingState->GetId());
	}


	//----------------------------

	m_gameManager->Initialize(); 

}




bool GameApplication::initWorkingDirectory()
{ 

	auto src_path = std::source_location::current();
	std::filesystem::path this_file = src_path.file_name(); //required conversion

	//set the current path 
	std::filesystem::current_path(this_file.parent_path().parent_path());
	this->m_workingDirectory = std::filesystem::current_path().string();

	std::cout << "Current working directory: " << this->m_workingDirectory << "\n";
	 

	return true;
}

bool GameApplication::initWindow()
{
	//create the main window:
	WindowCreateInfo createInfo;
	createInfo.width = m_width;
	createInfo.height = m_height;
	createInfo.title = m_title;
	m_mainWindow = WindowFactory::createWindow(createInfo);

	return (m_mainWindow != nullptr);
}

GameApplication* GameApplication::GetInstance()
{
	return s_instance;
}

std::string GameApplication::GetAssetFullPath(std::string assetName)
{
	return m_workingDirectory + "\\" + assetName;
}