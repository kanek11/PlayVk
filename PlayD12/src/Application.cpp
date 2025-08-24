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

		gTime->BeginFrame();

		auto& timeInfo = gTime->GetTimeInfo();
		float delta = (float)timeInfo.engineDelta;
		float FPS = 1.0f / delta;
		std::string text =
			std::format(" rDelta: {:.4f}", timeInfo.realDelta)
			+ std::format(" eDelta: {:.4f}", timeInfo.engineDelta)
			+ std::format(" eFPS: {:.2f}", FPS)
			+ std::format(" eTime: {:.4f}", timeInfo.engineTime)
			+ std::format(" simTime: {:.4f}", timeInfo.simTime);
		m_mainWindow->SetCustomWindowText(text);



		//todo: physics interpolation;  
		//=======
		m_mainWindow->onUpdate();

		m_inputSystem->OnUpdate();

		m_uiManager->RouteEvents();

		m_uiManager->Tick(delta);

		m_world->OnTick(delta);

		m_world->SyncGameToPhysics();


		//=======
		gTime->PumpFixedSteps();  


		//=======
		m_renderer->OnUpdate(delta);
		m_renderer->OnRender();


		//m_taskSystem.AddTask(
		//	"main",
		//	[=]() {
		//		m_mainWindow->onUpdate();

		//		m_inputSystem->OnUpdate();

		//		m_uiManager->RouteEvents();

		//		m_uiManager->Tick(delta);

		//		//todo: physics interpolation;  

		//		m_world->OnTick(delta);

		//		m_world->SyncGameToPhysics();
		//	},
		//	System::ETaskDomain::MainThread,
		//	{}
		//);



		//m_taskSystem.AddTask(
		//	"physics",
		//	[=]() {
		//		//std::cout << "dummy task\n" ;   
		//		gTime->PumpFixedSteps();
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
		// 
		// 
		m_world->EndFrame();

	}//while

	m_world->EndPlay();
}

void GameApplication::onBeginGame()
{
	//new: world update should comes before 
	auto globalLevel = m_world->CreateAndRegisterLevel<GlobalLevel>("global");
	m_world->SetPersistentLevel(globalLevel);
	//
	m_world->CreateAndRegisterLevel<GamePlayLevel>("gameplay"); 

	m_world->Init();
	m_world->BeginPlay();
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