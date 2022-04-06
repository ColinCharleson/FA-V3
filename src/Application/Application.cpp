#include "Application/Application.h"

#include <Windows.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Logging.h"
#include "Gameplay/InputEngine.h"
#include "Application/Timing.h"
#include <filesystem>
#include "Layers/GLAppLayer.h"
#include "Utils/FileHelpers.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ImGuiHelper.h"

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture1D.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/Sound.h"
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Gameplay/Components/Light.h"
#include "Gameplay/Components/ShadowCamera.h"

//Custom Components
#include <Gameplay\Components\ItemKeyBehaviour.h>
#include <Gameplay\Components\ItemBandageBehaviour.h>
#include <Gameplay\Components\ItemAmmoBehaviour.h>
#include <Gameplay\Components\DoorBehaviour.h>
#include <Gameplay\Components\SlimeBehaviour.h>
#include <Gameplay\Components\PauseScreenBehaviour.h>
#include <Gameplay\Components\MenuScreenBehaviour.h>
#include <Gameplay\Components\HealthBar.h>
#include <Gameplay\Components\HealthBar2HP.h>
#include <Gameplay\Components\HealthBar1HP.h>
#include <Gameplay\Components\WinScreenBehaviour.h>
#include <Gameplay\Components\LoseScreenBehaviour.h>
#include <Gameplay\Components\NormalAmmoCount.h>
#include <Gameplay\Components\BandageCount.h>
#include <Gameplay\Components\SpikeBehaviour.h>
#include <Gameplay\Components\Bolt.h>
#include <Gameplay\Components\AimPoint.h>
#include <Gameplay\Components\EnemyAI.h>
#include "Gameplay/Components/EnemyAiSpider.h"
#include "Gameplay/Components/EnemyAiGolem.h"
#include <Gameplay\Components\MorphMeshRenderer.h>
#include <Gameplay\Components\MorphAnimator.h>
#include "Gameplay/Physics/EnemyPath.h"
#include "Gameplay/Physics/EnemyPathCatMull.h"
#include "Gameplay/Physics/EnemyPathBezeir.h"
// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/Components/ComponentManager.h"

// Layers
#include "Layers/RenderLayer.h"
#include "Layers/InterfaceLayer.h"
#include "Layers/DefaultSceneLayer.h"
#include "Layers/LogicUpdateLayer.h"
#include "Layers/ImGuiDebugLayer.h"
#include "Layers/InstancedRenderingTestLayer.h"
#include "Layers/ParticleLayer.h"
#include "Layers/PostProcessingLayer.h"

Application* Application::_singleton = nullptr;
std::string Application::_applicationName = "INFR-2350U - DEMO";

float playerX, playerY;
bool onMenu = true;
bool onUiAmmo = false;
bool onUiHealth = false;
bool onUIBandage = false;
bool pausePressed;
bool gameWin = false;
bool gamePaused = false;
bool levelComplete = false;
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

Application::Application() :
	_window(nullptr),
	_windowSize({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}),
	_isRunning(false),
	_isEditor(true),
	_windowTitle("Forgotten Abyss"),
	_currentScene(nullptr),
	_targetScene(nullptr)
{ }

Application::~Application() = default; 

Application& Application::Get() {
	LOG_ASSERT(_singleton != nullptr, "Failed to get application! Get was called before the application was started!");
	return *_singleton;
}

void Application::Start(int argCount, char** arguments) {
	LOG_ASSERT(_singleton == nullptr, "Application has already been started!");
	_singleton = new Application();
	_singleton->_Run();
}

GLFWwindow* Application::GetWindow() { return _window; }

const glm::ivec2& Application::GetWindowSize() const { return _windowSize; }


const glm::uvec4& Application::GetPrimaryViewport() const {
	return _primaryViewport;
}

void Application::SetPrimaryViewport(const glm::uvec4& value) {
	_primaryViewport = value;
}

void Application::ResizeWindow(const glm::ivec2& newSize)
{
	_HandleWindowSizeChanged(newSize);
}

void Application::Quit() {
	_isRunning = false;
}

bool Application::LoadScene(const std::string& path) {
	if (std::filesystem::exists(path)) { 

		std::string manifestPath = std::filesystem::path(path).stem().string() + "-manifest.json";
		if (std::filesystem::exists(manifestPath)) {
			LOG_INFO("Loading manifest from \"{}\"", manifestPath);
			ResourceManager::LoadManifest(manifestPath);
		}

		Gameplay::Scene::Sptr scene = Gameplay::Scene::Load(path);
		LoadScene(scene);
		return scene != nullptr;
	}
	return false;
}

void Application::LoadScene(const Gameplay::Scene::Sptr& scene) {
	_targetScene = scene;
}

void Application::SaveSettings()
{
	std::filesystem::path appdata = getenv("APPDATA");
	std::filesystem::path settingsPath = appdata / _applicationName / "app-settings.json";

	if (!std::filesystem::exists(appdata / _applicationName)) {
		std::filesystem::create_directory(appdata / _applicationName);
	}

	FileHelpers::WriteContentsToFile(settingsPath.string(), _appSettings.dump(1, '\t'));
}

void Application::_Run()
{
	Sound soundplay;
	soundplay.init();
	soundplay.loadsound("Music", "Sounds/title_screen.wav", true);
	soundplay.playsound("Music");
	// TODO: Register layers
	_layers.push_back(std::make_shared<GLAppLayer>());
	_layers.push_back(std::make_shared<LogicUpdateLayer>());
	_layers.push_back(std::make_shared<RenderLayer>());
	_layers.push_back(std::make_shared<ParticleLayer>());
	_layers.push_back(std::make_shared<PostProcessingLayer>());
	_layers.push_back(std::make_shared<InterfaceLayer>());

	// If we're in editor mode, we add all the editor layers
	if (_isEditor) {
		_layers.push_back(std::make_shared<ImGuiDebugLayer>());
	}

	_layers.push_back(std::make_shared<DefaultSceneLayer>());

	// Either load the settings, or use the defaults
	_ConfigureSettings();

	// We'll grab these since we'll need them!
	_windowSize.x = JsonGet(_appSettings, "window_width", DEFAULT_WINDOW_WIDTH);
	_windowSize.y = JsonGet(_appSettings, "window_height", DEFAULT_WINDOW_HEIGHT);

	// By default, we want our viewport to be the whole screen
	_primaryViewport = { 0, 0, _windowSize.x, _windowSize.y };

	// Register all component and resource types
	_RegisterClasses();


	// Load all layers
	_Load();

	// Grab current time as the previous frame
	double lastFrame =  glfwGetTime();

	// Done loading, app is now running!
	_isRunning = true;

	// Infinite loop as long as the application is running
	while (_isRunning) {

		soundplay.update();
		Sleep(16);

		bool startPlaying = false;
		if (InputEngine::IsKeyDown(GLFW_KEY_SPACE))
		{

			onMenu = false;
			onUiHealth = true;
			onUIBandage = true;
			onUiAmmo = true;
			startPlaying = true;
		}


		if (InputEngine::IsKeyDown(GLFW_KEY_W))
		{
			playerX = _currentScene->MainCamera->GetGameObject()->GetPosition().x;
			playerY = _currentScene->MainCamera->GetGameObject()->GetPosition().y;
		}
		if (InputEngine::IsKeyDown(GLFW_KEY_A))
		{
			playerX = _currentScene->MainCamera->GetGameObject()->GetPosition().x;
			playerY = _currentScene->MainCamera->GetGameObject()->GetPosition().y;
		}
		if (InputEngine::IsKeyDown(GLFW_KEY_S))
		{
			playerX = _currentScene->MainCamera->GetGameObject()->GetPosition().x;
			playerY = _currentScene->MainCamera->GetGameObject()->GetPosition().y;
		}
		if (InputEngine::IsKeyDown(GLFW_KEY_D))
		{
			playerX = _currentScene->MainCamera->GetGameObject()->GetPosition().x;
			playerY = _currentScene->MainCamera->GetGameObject()->GetPosition().y;

		}


		if (levelComplete)
		{/*
			if (progressScore < 1)
			{
				levelComplete = false;
				hasKey = false;
				RoomFunction();
			}
			else*/
			{
				gameWin = true;
			}
		}

		bool pressed = glfwGetKey(_window, GLFW_KEY_ESCAPE);
		if (pressed)
		{
			if (gameWin == false)
			{
				if (pausePressed == false)
				{
					gamePaused = !gamePaused;
				}
				pausePressed = pressed;


			}
			onUiHealth = false;
			onUIBandage = false;
			onUiAmmo = false;
		}
		else
		{
			pausePressed = false;
		}

		if (startPlaying == true)
		{
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		// Handle scene switching
		if (_targetScene != nullptr) {
			_HandleSceneChange();
		}

		// Receive events like input and window position/size changes from GLFW
		glfwPollEvents();

		// Handle closing the app via the close button
		if (glfwWindowShouldClose(_window)) {
			_isRunning = false;
		}

		// Grab the timing singleton instance as a reference
		Timing& timing = Timing::_singleton;

		// Figure out the current time, and the time since the last frame
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);
		float scaledDt = dt * timing._timeScale;

		// Update all timing values
		timing._unscaledDeltaTime = dt;
		timing._deltaTime = scaledDt;
		timing._timeSinceAppLoad += scaledDt;
		timing._unscaledTimeSinceAppLoad += dt;
		timing._timeSinceSceneLoad += scaledDt;
		timing._unscaledTimeSinceSceneLoad += dt;

		ImGuiHelper::StartFrame();

		// Core update loop
		if (_currentScene != nullptr) {
			_Update();
			_LateUpdate();
			_PreRender();
			_RenderScene(); 
			_PostRender();
		}

		// Store timing for next loop
		lastFrame = thisFrame;

		InputEngine::EndFrame();
		ImGuiHelper::EndFrame();

		glfwSwapBuffers(_window);

	}

	// Unload all our layers
	_Unload();
}

void Application::_RegisterClasses()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture1D>();
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<Texture3D>();
	ResourceManager::RegisterType<TextureCube>();
	ResourceManager::RegisterType<ShaderProgram>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();
	ResourceManager::RegisterType<Font>();
	ResourceManager::RegisterType<Framebuffer>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<TriggerVolumeEnterBehaviour>();
	ComponentManager::RegisterType<SimpleCameraControl>();
	ComponentManager::RegisterType<RectTransform>();
	ComponentManager::RegisterType<GuiPanel>();
	ComponentManager::RegisterType<GuiText>();
	ComponentManager::RegisterType<ParticleSystem>();
	ComponentManager::RegisterType<Light>();
	ComponentManager::RegisterType<ShadowCamera>();

	//Custom Components 
	ComponentManager::RegisterType<AimPoint>();
	ComponentManager::RegisterType<Bolt>();
	ComponentManager::RegisterType<EnemyBehaviour>();
	ComponentManager::RegisterType<EnemyBehaviourSpider>();
	ComponentManager::RegisterType<EnemyBehaviourGolem>();
	ComponentManager::RegisterType<EnemyPath>();
	ComponentManager::RegisterType<EnemyPathCatMull>();
	ComponentManager::RegisterType<EnemyPathBezeir>();
	ComponentManager::RegisterType<KeyBehaviour>();
	ComponentManager::RegisterType<AmmoBehaviour>();
	ComponentManager::RegisterType<BandageBehaviour>();
	ComponentManager::RegisterType<SpikeBehaviour>();
	ComponentManager::RegisterType<SlimeBehaviour>();
	ComponentManager::RegisterType<DoorBehaviour>();
	ComponentManager::RegisterType<PauseScreen>();
	ComponentManager::RegisterType<MenuScreen>();
	ComponentManager::RegisterType<WinScreen>();
	ComponentManager::RegisterType<LoseScreen>();
	ComponentManager::RegisterType<MorphMeshRenderer>();
	ComponentManager::RegisterType<HealthBehaviour>();
	ComponentManager::RegisterType<HealthBehaviour2>();
	ComponentManager::RegisterType<HealthBehaviour1>();
	ComponentManager::RegisterType<NormalAmmo>();
	ComponentManager::RegisterType<BandageCount>();
}

void Application::_Load() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnAppLoad)) {
			layer->OnAppLoad(_appSettings);
		}
	}

	// Pass the window to the input engine and let it initialize itself
	InputEngine::Init(_window);
	
	// Initialize our ImGui helper
	ImGuiHelper::Init(_window);

	GuiBatcher::SetWindowSize(_windowSize);
}

void Application::_Update() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnUpdate)) {
			layer->OnUpdate();
		}
	}
}

void Application::_LateUpdate() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnLateUpdate)) {
			layer->OnLateUpdate();
		}
	}
}

void Application::_PreRender()
{
	glm::ivec2 size ={ 0, 0 };
	glfwGetWindowSize(_window, &size.x, &size.y);
	glViewport(0, 0, size.x, size.y);
	glScissor(0, 0, size.x, size.y);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnPreRender)) {
			layer->OnPreRender();
		}
	}
}

void Application::_RenderScene() {

	Framebuffer::Sptr result = nullptr;
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnRender)) {
			layer->OnRender(result);
		}
	}
}

void Application::_PostRender() {
	// Note that we use a reverse iterator for post render
	for (auto it = _layers.begin(); it != _layers.end(); it++) {
		const auto& layer = *it;
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnPostRender)) {
			layer->OnPostRender();
		}
	}
}

//// We can use the application's viewport to set our OpenGL viewport, as well as clip rendering to that area
//const glm::uvec4& viewport = GetPrimaryViewport();
//glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
//glScissor(viewport.x, viewport.y, viewport.z, viewport.w);
//
//// If we have a final output, blit it to the screen
//if (_renderOutput != nullptr)
//{
//	_renderOutput->Unbind();
//
//	glm::ivec2 windowSize = _windowSize;
//	if (_isEditor)
//	{
//		glfwGetWindowSize(_window, &windowSize.x, &windowSize.y);
//	}
//	//glViewport(0, 0, windowSize.x, windowSize.y);
//	glm::ivec4 viewportMinMax = { viewport.x, viewport.y, viewport.x + viewport.z, viewport.y + viewport.w };
//
//	_renderOutput->Bind(FramebufferBinding::Read);
//	glBindFramebuffer(*FramebufferBinding::Write, 0);
//	Framebuffer::Blit({ 0, 0, _renderOutput->GetWidth(), _renderOutput->GetHeight() }, viewportMinMax, BufferFlags::All, MagFilter::Nearest);
//
//}

void Application::_Unload() {
	// Note that we use a reverse iterator for unloading
	for (auto it = _layers.crbegin(); it != _layers.crend(); it++) {
		const auto& layer = *it;
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnAppUnload)) {
			layer->OnAppUnload();
		}
	}

	// Clean up ImGui
	ImGuiHelper::Cleanup();
}

void Application::_HandleSceneChange() {
	// If we currently have a current scene, let the layers know it's being unloaded
	if (_currentScene != nullptr) {
		// Note that we use a reverse iterator, so that layers are unloaded in the opposite order that they were loaded
		for (auto it = _layers.crbegin(); it != _layers.crend(); it++) {
			const auto& layer = *it;
			if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnSceneUnload)) {
				layer->OnSceneUnload();
			}
		}
	}

	_currentScene = _targetScene;
	
	// Let the layers know that we've loaded in a new scene
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnSceneLoad)) {
			layer->OnSceneLoad();
		}
	}

	// Wake up all game objects in the scene
	_currentScene->Awake();

	// If we are not in editor mode, scenes play by default
	if (!_isEditor) {
		_currentScene->IsPlaying = true;
	}

	_targetScene = nullptr;
}

void Application::_HandleWindowSizeChanged(const glm::ivec2& newSize) {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnWindowResize)) {
			layer->OnWindowResize(_windowSize, newSize);
		}
	}
	_windowSize = newSize;
	_primaryViewport = { 0, 0, newSize.x, newSize.y };
}

void Application::_ConfigureSettings() {
	// Start with the defaul application settings
	_appSettings = _GetDefaultAppSettings();

	// We'll store our settings in the %APPDATA% directory, under our application name
	std::filesystem::path appdata = getenv("APPDATA");
	std::filesystem::path settingsPath = appdata / _applicationName / "app-settings.json";

	// If the settings file exists, we can load it in!
	if (std::filesystem::exists(settingsPath)) {
		// Read contents and parse into a JSON blob
		std::string content = FileHelpers::ReadFile(settingsPath.string());
		nlohmann::json blob = nlohmann::json::parse(content);

		// We use merge_patch so that we can keep our defaults if they are missing from the file!
		_appSettings.merge_patch(blob);
	}
	// If the file does not exist, save the default application settings to the path
	else {
		SaveSettings();
	}
}

nlohmann::json Application::_GetDefaultAppSettings()
{
	nlohmann::json result ={};

	for (const auto& layer : _layers) {
		if (!layer->Name.empty()) {
			result[layer->Name] = layer->GetDefaultConfig();
		}
		else {
			LOG_WARN("Unnamed layer! Injecting settings into global namespace, may conflict with other layers!");
			result.merge_patch(layer->GetDefaultConfig());
		}
	}

	result["window_width"]  = DEFAULT_WINDOW_WIDTH;
	result["window_height"] = DEFAULT_WINDOW_HEIGHT;
	return result;
}

