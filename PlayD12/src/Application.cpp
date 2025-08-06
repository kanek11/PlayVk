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
	//owningWorld->physicsScene = new PhysicsScene();
	//owningWorld->physicsScene->OnInit();

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
	m_world = new UWorld();

}

void GameApplication::onDestroy()
{
	m_renderer->OnDestroy();
}

void GameApplication::run()
{


	//gTime.RegisterFixedFrame([=](float delta) {
	//	owningWorld->physicsScene->Tick(delta);
	//	});

	//todo:  exit condition could be more complex.
	while (!m_mainWindow->shouldClose()) {
		//std::cout << "tick main loop" << '\n';

		gTime.BeginFrame();

		auto& timeInfo = gTime.GetTimeInfo();
		float delta = (float)timeInfo.engineDelta;
		float FPS = 1.0f / delta;
		std::string text =
			std::format(" rDelta: {:.4f}", timeInfo.realDelta)
			+ std::format(" eDelta: {:.4f}", timeInfo.engineDelta)
			+ std::format(" eFPS: {:.2f}", FPS)
			+ std::format(" eTime: {:.4f}", timeInfo.engineTime)
			+ std::format(" simTime: {:.4f}", timeInfo.simTime);
		m_mainWindow->SetCustomWindowText(text);

		//
		m_mainWindow->onUpdate();

		m_inputSystem->OnUpdate();

		m_uiManager->ProcessEvents();

		//owningWorld->physicsScene->Tick(0.016f);


		if (m_inputSystem->IsKeyJustPressed(KeyCode::Space)) {
			//std::cout << "app: A is pressed" << '\n';
			gTime.TogglePaused();
		}
		else if (m_inputSystem->IsKeyJustReleased(KeyCode::L)) {
			//std::cout << "app: A is released" << '\n';
			gTime.AdvanceFixedSteps();
			gTime.AdvanceFrames();
		}

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

	/*	if (m_inputSystem->IsKeyJustPressed(KeyCode::Num1)) {
			std::cout << "app: 1 is pressed" << '\n';
			m_gameManager->TransitState(GameStateId::Playing);
		}
		else
			if (m_inputSystem->IsKeyJustPressed(KeyCode::Num2)) {
				std::cout << "app: 2 is pressed" << '\n';
				m_gameManager->TransitState(GameStateId::MainMenu);
			}*/

			//tick level transition;
		m_gameManager->Update(delta);


		m_world->SyncPhysicsToGame();

		//todo: physics interpolation; 
		//tick level>actor> component
		m_world->OnTick(delta);

		m_world->SyncGameToPhysics();

		gTime.PumpFixedSteps();

		m_renderer->OnUpdate(delta);
		m_renderer->OnRender();


		//m_taskSystem.AddTask(
		//	"physics",
		//	[=]() {
		//		//std::cout << "dummy task\n" ;   
		//		gTime.PumpFixedSteps();
		//	},
		//	System::ETaskDomain::PhysicsThread,
		//	{}
		//);

		//m_taskSystem.AddTask(
		//"renderThread",
		//[=]() {
		//	//std::cout << "dummy task\n" ;   
		//	m_renderer->OnUpdate(delta);
		//	m_renderer->OnRender();
		//},
		//System::ETaskDomain::RenderThread,
		//{}
		//);

		//m_taskSystem.ExecuteAll();  

	}

	m_world->EndPlay();
}

void GameApplication::onBeginGame()
{
	//new: world update should comes before
	m_world->Init();
	m_world->BeginPlay();


	m_world->RegisterLevel("gameplay", CreateShared<GamePlayLevel>());
	m_world->RegisterLevel("mainMenu", CreateShared<MainMenuLevel>());


	if (auto mainMenuState = m_gameManager->Register<MainMenuState>()) {
		mainMenuState->OnEnter.Add(
			[&]() {
				std::cout << " enter mainMenu state" << "\n";
				m_world->TransitLevel("mainMenu");
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

				m_world->TransitLevel("gameplay");
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