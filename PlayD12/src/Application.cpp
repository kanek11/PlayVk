#include "PCH.h"
#include "Application.h"

GameApplication* GameApplication::s_instance = nullptr;

 
GameApplication::GameApplication()
{
	s_instance = this;
}

void GameApplication::onInit()
{
	//init working directory:
	if (!initWorkingDirectory())
	{
		throw std::runtime_error("Failed to initialize working directory");
	}
	 
	//test:
	std::cout << this->GetAssetFullPath("Shaders/shaders.hlsl") << "\n";


	if (!initWindow()) {

		throw std::runtime_error("Failed to initialize window");

	}

	auto wideTitle = std::wstring(m_title.begin(), m_title.end());



	m_physicsScene = new PhysicsScene();
	m_physicsScene->OnInit();

	m_renderer = new D3D12HelloRenderer(m_width, m_height, wideTitle.c_str(), m_mainWindow);
	m_renderer->OnInit();

	m_inputSystem = new InputSystem();

	//m_mainWindow->InitInputSource(m_inputSystem);

}

void GameApplication::onDestroy()
{
	m_renderer->OnDestroy();
}

void GameApplication::run()
{
	//todo:  exit condition could be more complex.
	while (!m_mainWindow->shouldClose()) {
		m_mainWindow->onUpdate();
		 
		m_physicsScene->Tick(0.016f);

		m_renderer->OnUpdate(0.016f);
		m_renderer->OnRender();

		m_inputSystem->OnUpdate();
		if (m_inputSystem->IsKeyJustPressed(KeyCode::A)) {
			std::cout << "app: A is pressed" << '\n';
		} 
		else if (m_inputSystem->IsKeyJustReleased(KeyCode::A)) {
			std::cout << "app: A is released" << '\n';
		} 
		else if (m_inputSystem->IsKeyDown(KeyCode::A)) {
			//std::cout << "app: A is down" << '\n';
		}
		else if (m_inputSystem->IsKeyUp(KeyCode::A)) {
			//std::cout << "app: A is up" << '\n';
		}
		 
	}
		
	 
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
