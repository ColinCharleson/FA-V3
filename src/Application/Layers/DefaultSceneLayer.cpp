#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL 
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/Light.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
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

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Application/Layers/ImGuiDebugLayer.h"
#include "Application/Windows/DebugWindow.h"
#include "Gameplay/Components/ShadowCamera.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

float boltX, boltY, boltZ;
bool arrowOut = false;
bool canShoot = true;
bool hasKey = false, slimeSlow = false;
int ammoCount = 5, playerHealth = 3, bandageCount = 0;



int roomType, progressScore;
glm::quat currentRot;
glm::vec3 forward;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		 
		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");  

		// Our foliage shader which manipulates the vertices of the mesh
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});  
		foliageShader->SetDebugName("Foliage");   

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },  
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing"); 

		// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");

	
		

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr shipMesh   = ResourceManager::CreateAsset<MeshResource>("fenrir.obj");

		// Load in some textures
		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);
		
		// Load in some textures
		MeshResource::Sptr spiderMesh = ResourceManager::CreateAsset<MeshResource>("SpiderMesh.obj");
		MeshResource::Sptr chestMesh = ResourceManager::CreateAsset<MeshResource>("Chest.obj"); //1
		Texture2D::Sptr    chestTexture = ResourceManager::CreateAsset<Texture2D>("textures/ChestTex.png");
		MeshResource::Sptr keyMesh = ResourceManager::CreateAsset<MeshResource>("Key.obj"); //2
		Texture2D::Sptr    keyTexture = ResourceManager::CreateAsset<Texture2D>("textures/KeyTex.png");
		MeshResource::Sptr bandageMesh = ResourceManager::CreateAsset<MeshResource>("Bandage.obj"); //3
		Texture2D::Sptr    bandageTexture = ResourceManager::CreateAsset<Texture2D>("textures/BandageTex.png");
		MeshResource::Sptr golemMesh = ResourceManager::CreateAsset<MeshResource>("GolemMesh.obj");
		MeshResource::Sptr bowMesh = ResourceManager::CreateAsset<MeshResource>("Crossbow.obj");
		Texture2D::Sptr    bowTexture = ResourceManager::CreateAsset<Texture2D>("textures/CrossbowTex.png");
		MeshResource::Sptr skeletonMesh = ResourceManager::CreateAsset<MeshResource>("SkeletonMesh.obj");
		Texture2D::Sptr    spiderTexture = ResourceManager::CreateAsset<Texture2D>("textures/SpiderTex.png");
		Texture2D::Sptr    spiderTextureEmissive = ResourceManager::CreateAsset<Texture2D>("textures/SpiderEmissive.png");
		MeshResource::Sptr torchMesh = ResourceManager::CreateAsset<MeshResource>("Torch.obj");  //4
		Texture2D::Sptr    torchTexture = ResourceManager::CreateAsset<Texture2D>("textures/TorchTex.png");
		MeshResource::Sptr arrowMesh = ResourceManager::CreateAsset<MeshResource>("ArrowPick.obj");  // 5
		MeshResource::Sptr boltMesh = ResourceManager::CreateAsset<MeshResource>("ArrowBolt3.obj");  //6
		Texture2D::Sptr    arrowTexture = ResourceManager::CreateAsset<Texture2D>("textures/BakedArrow.png");
		Texture2D::Sptr    golemTexture = ResourceManager::CreateAsset<Texture2D>("textures/GolemTex.png");
		Texture2D::Sptr    golemTextureEmissive = ResourceManager::CreateAsset<Texture2D>("textures/GolemTexEmmisive.png");
		Texture2D::Sptr    skeletonTexture = ResourceManager::CreateAsset<Texture2D>("textures/SkeletonTex.png");
		Texture2D::Sptr    skeletonTextureEmissive = ResourceManager::CreateAsset<Texture2D>("textures/SkeletonTexEmissive.png");
		
		MeshResource::Sptr wallMesh = ResourceManager::CreateAsset<MeshResource>("WallColin.obj");  //7
		MeshResource::Sptr pillarMesh = ResourceManager::CreateAsset<MeshResource>("PillarLip.obj");  //7

		MeshResource::Sptr SpikeTrap = ResourceManager::CreateAsset<MeshResource>("Spike.obj"); //10
		Texture2D::Sptr    spikeTexture = ResourceManager::CreateAsset<Texture2D>("textures/Spike.png");
		MeshResource::Sptr slimeTrap = ResourceManager::CreateAsset<MeshResource>("Slime.obj"); //11
		Texture2D::Sptr    slimeTexture = ResourceManager::CreateAsset<Texture2D>("textures/Slime.png");

		Texture2D::Sptr    wallTexture = ResourceManager::CreateAsset<Texture2D>("textures/WallTexture.png");
		Texture2D::Sptr    wallTexture2 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar2.png");
		Texture2D::Sptr    wallTexture3 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar3.png");
		Texture2D::Sptr    wallTexture4 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar4.png");
		Texture2D::Sptr    wallTexture5 = ResourceManager::CreateAsset<Texture2D>("textures/WallTextureVar5.png");
		MeshResource::Sptr doormesh = ResourceManager::CreateAsset<MeshResource>("DoorModel.obj"); //12
		Texture2D::Sptr    doorTexture = ResourceManager::CreateAsset<Texture2D>("textures/DoorTexture.png");
		Texture2D::Sptr    floorTexture = ResourceManager::CreateAsset<Texture2D>("textures/Base.png");
		MeshResource::Sptr BedMesh = ResourceManager::CreateAsset<MeshResource>("Bed.obj");
		Texture2D::Sptr    Bed = ResourceManager::CreateAsset<Texture2D>("textures/BedTex.png");

		Texture2D::Sptr    healthBar = ResourceManager::CreateAsset<Texture2D>("textures/OperationHealthFull.png");
		Texture2D::Sptr    healthBar2 = ResourceManager::CreateAsset<Texture2D>("textures/Operation1ThirdHealth.png");
		Texture2D::Sptr    healthBar1 = ResourceManager::CreateAsset<Texture2D>("textures/Operation2thirdsHealth.png");
		Texture2D::Sptr    ammoHUD = ResourceManager::CreateAsset<Texture2D>("textures/AmmoHUD.png");

		MeshResource::Sptr ChainModel = ResourceManager::CreateAsset<MeshResource>("Chain.obj"); // 13
		Texture2D::Sptr    ChainTexture = ResourceManager::CreateAsset<Texture2D>("textures/ChainTex.png");
		MeshResource::Sptr Chain2Model = ResourceManager::CreateAsset<MeshResource>("Chain2.obj"); // 14
		Texture2D::Sptr    Chain2Texture = ResourceManager::CreateAsset<Texture2D>("textures/Chain2Tex.png");
		MeshResource::Sptr BonesModel = ResourceManager::CreateAsset<MeshResource>("Bones.obj"); // 15
		Texture2D::Sptr    BonesTexture = ResourceManager::CreateAsset<Texture2D>("textures/BoneTex.png");
		MeshResource::Sptr BarrelModel = ResourceManager::CreateAsset<MeshResource>("Barrel.obj"); // 17
		Texture2D::Sptr    BarrelTexture = ResourceManager::CreateAsset<Texture2D>("textures/BarrelTex.png");

		Texture2D::Sptr    pausePNG = ResourceManager::CreateAsset<Texture2D>("textures/UIPause2.png");
		Texture2D::Sptr    menuPNG = ResourceManager::CreateAsset<Texture2D>("textures/UIMenu2.png");
		Texture2D::Sptr    losePNG = ResourceManager::CreateAsset<Texture2D>("textures/UILose2.png");
		Texture2D::Sptr    winPNG = ResourceManager::CreateAsset<Texture2D>("textures/UIWin2.png");
		Texture2D::Sptr    Crosshair = ResourceManager::CreateAsset<Texture2D>("textures/CH.png");
		Texture2D::Sptr    Bandage = ResourceManager::CreateAsset<Texture2D>("textures/Bandaid.png");
		// Load some images for drag n' drop
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight.png");
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight-2.png");
		ResourceManager::CreateAsset<Texture2D>("textures/light_projection.png");

		

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidGrey[3] = { 0.5f, 0.5f, 0.5f };
		float solidBlack[3] = { 0.0f, 0.0f, 0.0f };
		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };

		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		});
		  
		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>();  

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		Material::Sptr golemMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			golemMaterial->Name = "golemMaterial";
			golemMaterial->Set("u_Material.AlbedoMap", golemTexture);
			golemMaterial->Set("u_Material.EmissiveMap", golemTextureEmissive);
			golemMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr chainMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			chainMaterial->Name = "chainMaterial";
			chainMaterial->Set("u_Material.AlbedoMap", ChainTexture);
			chainMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr chain2Material = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			chain2Material->Name = "chain2Material";
			chain2Material->Set("u_Material.AlbedoMap", Chain2Texture);
			chain2Material->Set("u_Material.NormalMap", normalMap);
		}


		Material::Sptr bonesMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			bonesMaterial->Name = "bonesMaterial";
			bonesMaterial->Set("u_Material.AlbedoMap", BonesTexture);
			bonesMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr spikeMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			spikeMaterial->Name = "spikeMaterial";
			spikeMaterial->Set("u_Material.AlbedoMap", spikeTexture);
			spikeMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr slimeMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			slimeMaterial->Name = "spikeMaterial";
			slimeMaterial->Set("u_Material.AlbedoMap", slimeTexture);
			slimeMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr roofMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/Ceiling1/plane_DefaultMaterial_BaseColor.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/Ceiling1/plane_DefaultMaterial_Normal.png");
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/Ceiling1/plane_DefaultMaterial_Roughness.png");

			roofMat->Name = "roofMat";
			roofMat->Set("u_Material.AlbedoMap", diffuseMap);
			roofMat->Set("u_Material.NormalMap", normalMap);
			roofMat->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarOne = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall1/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall1/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall1/wall_Wall_Material_BaseColor.1001.png");

			wallMaterialVarOne->Name = "wallMaterialVarOne";
			wallMaterialVarOne->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarOne->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarOne->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarTwo = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall2/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall2/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall2/wall_Wall_Material_BaseColor.1001.png");
			
			wallMaterialVarTwo->Name = "wallMaterialVarTwo";
			wallMaterialVarTwo->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarTwo->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarTwo->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarThree = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall3/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall3/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall3/wall_Wall_Material_BaseColor.1001.png");
			
			wallMaterialVarThree->Name = "wallMaterialVarThree";
			wallMaterialVarThree->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarThree->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarThree->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarFour = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall4/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall4/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall4/wall_Wall_Material_BaseColor.1001.png");
			
			wallMaterialVarFour->Name = "wallMaterialVarFour";
			wallMaterialVarFour->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarFour->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarFour->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarFive = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall5/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall5/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall5/wall_Wall_Material_BaseColor.1001.png");
			
			wallMaterialVarFive->Name = "wallMaterialVarFive";
			wallMaterialVarFive->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarFive->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarFive->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr wallMaterialVarSix = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/wall6/wall_Wall_Material_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/wall6/wall_Wall_Material_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/wall6/wall_Wall_Material_BaseColor.1001.png");
			
			wallMaterialVarSix->Name = "wallMaterialVarSix";
			wallMaterialVarSix->Set("u_Material.AlbedoMap", diffuseMap);
			wallMaterialVarSix->Set("u_Material.NormalMap", normalMap);
			wallMaterialVarSix->Set("s_Heightmap", displacementMap);
		}
		Material::Sptr pillarMaterialVarOne = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/Pillar1/PillarNEW_PillarMat1_Roughness.1001.png");
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/Pillar1/PillarNEW_PillarMat1_Normal.1001.png");
			Texture2D::Sptr diffuseMap = ResourceManager::CreateAsset<Texture2D>("textures/Pillar1/PillarNEW_PillarMat1_BaseColor.1001.png");
			
			pillarMaterialVarOne->Name = "pillarMaterialVarOne";
			pillarMaterialVarOne->Set("u_Material.AlbedoMap", diffuseMap);
			pillarMaterialVarOne->Set("u_Material.NormalMap", normalMap);
			pillarMaterialVarOne->Set("s_Heightmap", displacementMap);
		}

		Material::Sptr chestMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			chestMaterial->Name = "chainMaterial";
			chestMaterial->Set("u_Material.AlbedoMap", chestTexture);
			chestMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr doorMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");

			doorMaterial->Name = "doorMaterial";
			doorMaterial->Set("u_Material.AlbedoMap", doorTexture);
			doorMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr spiderMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			spiderMaterial->Name = "spiderMaterial";
			spiderMaterial->Set("u_Material.AlbedoMap", spiderTexture);
			spiderMaterial->Set("u_Material.EmissiveMap", spiderTextureEmissive);
			spiderMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr arrowMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");

			arrowMaterial->Name = "arrowMaterial";
			arrowMaterial->Set("u_Material.AlbedoMap", arrowTexture);
			arrowMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr keyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			keyMaterial->Name = "keyMaterial";
			keyMaterial->Set("u_Material.AlbedoMap", keyTexture);
			keyMaterial->Set("u_Material.EmissiveMap", keyTexture);
			keyMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr bandageMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");

			bandageMaterial->Name = "bandageMaterial";
			bandageMaterial->Set("u_Material.AlbedoMap", bandageTexture);
			bandageMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr bowMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");

			bowMat->Name = "bowMat";
			bowMat->Set("u_Material.AlbedoMap", bowTexture);
			bowMat->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr skeletonMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/DefMap.png");
			skeletonMaterial->Name = "skeletonMaterial";
			skeletonMaterial->Set("u_Material.AlbedoMap", skeletonTexture);
			skeletonMaterial->Set("u_Material.EmissiveMap", skeletonTextureEmissive);
			skeletonMaterial->Set("u_Material.NormalMap", normalMap);
		}

		Material::Sptr floorMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap = ResourceManager::CreateAsset<Texture2D>("textures/Ceiling1/plane_DefaultMaterial_Normal.png");
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/Ceiling1/plane_DefaultMaterial_Roughness.png");

			floorMaterial->Name = "floorMaterial";
			floorMaterial->Set("u_Material.AlbedoMap", floorTexture);
			floorMaterial->Set("u_Material.NormalMap", normalMap);
			floorMaterial->Set("s_Heightmap", displacementMap);

		}
		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

	/*	for (int ix = 0; ix < 50; ix++)
		{
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPostion(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}*/

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion(glm::vec3(1.0f, -1.0f, 1.0f));
			camera->LookAt(glm::vec3(1, -12, 1));

			camera->Add<SimpleCameraControl>();

		//	Camera::Sptr cam = camera->Add<Camera>();

			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;

			RenderComponent::Sptr renderer = camera->Add<RenderComponent>();
			renderer->SetMesh(bowMesh);
			renderer->SetMaterial(bowMat);

			RigidBody::Sptr PlayerTrigger = camera->Add<RigidBody>(RigidBodyType::Dynamic);
			PlayerTrigger->AddCollider(SphereCollider::Create(0.75));
			PlayerTrigger->SetLinearDamping(2.0f);
		}
		GameObject::Sptr loadedBolt = scene->CreateGameObject("fake Bolt");
		{
			// Set position in the scene
			loadedBolt->SetPostion(glm::vec3(0, -0.25f, -0.21f));
			loadedBolt->SetRotation(glm::vec3(0, 0, 0));
			loadedBolt->SetScale(glm::vec3(0.3f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = loadedBolt->Add<RenderComponent>();
			renderer->SetMesh(boltMesh);
			renderer->SetMaterial(arrowMaterial);

			loadedBolt->Add<AimPoint>();
			camera->AddChild(loadedBolt);
		}
		GameObject::Sptr playerBolt = scene->CreateGameObject("Bolt");
		{
			// Set position in the scene
			playerBolt->SetPostion(glm::vec3(1.f, -1.25f, 0.76f));
			playerBolt->SetRotation(glm::vec3(-90.f, 0, 0));
			playerBolt->SetScale(glm::vec3(0.3f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = playerBolt->Add<RenderComponent>();
			renderer->SetMesh(boltMesh);
			renderer->SetMaterial(arrowMaterial);

			playerBolt->Add<Bolt>();
		}
		// Set up all our sample objects
		GameObject::Sptr floor = scene->CreateGameObject("Floor");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = floor->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(floorMaterial);

			RigidBody::Sptr floorRB = floor->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(PlaneCollider::Create());
		}


		GameObject::Sptr deccor = scene->CreateGameObject("DeccorationGroup");
		GameObject::Sptr impo = scene->CreateGameObject("Important Items");

		/*GameObject::Sptr Beding = scene->CreateGameObject("Bed");
		{

			Beding->SetPostion(glm::vec3(2.550f, -0.730f, -0.020f));
			Beding->SetRotation(glm::vec3(90.0f, 0.0f, -106.0f));
			Beding->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Beding->Add<RenderComponent>();
			renderer->SetMesh(BedMesh);
			renderer->SetMaterial(BedMaterial);
			deccor->AddChild(Beding);

		}*/
		GameObject::Sptr roof = scene->CreateGameObject("Roof");
		{
			roof->SetPostion(glm::vec3(0.0f, 0.0f, 2.5f));
			roof->SetRotation(glm::vec3(180.0f, 0.0f, 0.0f));
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = roof->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(roofMat);
			impo->AddChild(roof);
		}
		GameObject::Sptr chestKey = scene->CreateGameObject("Chest1");
		{
			// Set position in the scene
			chestKey->SetPostion(glm::vec3(16.5f, -1.0f, 0.0f));
			chestKey->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			chestKey->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestKey->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestKey->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
			deccor->AddChild(chestKey);
		}
		GameObject::Sptr keyObj = scene->CreateGameObject("key");
		{
			// Set position in the scene
			keyObj->SetPostion(glm::vec3(16.5f, -1.0f, 0.0f));
			keyObj->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			keyObj->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = keyObj->Add<RenderComponent>();
			renderer->SetMesh(keyMesh);
			renderer->SetMaterial(keyMaterial);

			keyObj->Add<KeyBehaviour>();
			impo->AddChild(keyObj);
		}
		GameObject::Sptr chestBandage = scene->CreateGameObject("Chest2");
		{
			// Set position in the scene
			chestBandage->SetPostion(glm::vec3(5.0f, -9.5f, 0.0f));
			chestBandage->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			chestBandage->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestBandage->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestBandage->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
			deccor->AddChild(chestBandage);
		}
		GameObject::Sptr bandageObj = scene->CreateGameObject("bandage");
		{
			// Set position in the scene
			bandageObj->SetPostion(glm::vec3(5.0f, -9.5f, 0.0f));
			bandageObj->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			bandageObj->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = bandageObj->Add<RenderComponent>();
			renderer->SetMesh(bandageMesh);
			renderer->SetMaterial(bandageMaterial);

			bandageObj->Add<BandageBehaviour>();
			impo->AddChild(bandageObj);
		}
		GameObject::Sptr chestAmmo1 = scene->CreateGameObject("Chest3");
		{
			// Set position in the scene
			chestAmmo1->SetPostion(glm::vec3(-6.5f, -14.0f, 0.0f));
			chestAmmo1->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			chestAmmo1->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestAmmo1->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestAmmo1->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.1f, 1, 0.1f)));
			deccor->AddChild(chestAmmo1);
		}
		GameObject::Sptr Ammo1 = scene->CreateGameObject("ammo pickup 1");
		{
			// Set position in the scene
			Ammo1->SetPostion(glm::vec3(-6.5f, -14.0f, 0.0f));
			Ammo1->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			Ammo1->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Ammo1->Add<RenderComponent>();
			renderer->SetMesh(arrowMesh);
			renderer->SetMaterial(arrowMaterial);

			Ammo1->Add<AmmoBehaviour>();
			impo->AddChild(Ammo1);
		}
		GameObject::Sptr chestAmmo2 = scene->CreateGameObject("Chest4");
		{
			// Set position in the scene
			chestAmmo2->SetPostion(glm::vec3(11.5f, -14.0f, 0.0f));
			chestAmmo2->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			chestAmmo2->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chestAmmo2->Add<RenderComponent>();
			renderer->SetMesh(chestMesh);
			renderer->SetMaterial(chestMaterial);

			RigidBody::Sptr floorRB = chestAmmo2->Add<RigidBody>(RigidBodyType::Static);
			floorRB->AddCollider(BoxCollider::Create(glm::vec3(0.3f, 1, 0.3f)));
			deccor->AddChild(chestAmmo2);
		}
		GameObject::Sptr Ammo2 = scene->CreateGameObject("ammo pickup2");
		{
			// Set position in the scene
			Ammo2->SetPostion(glm::vec3(11.5f, -14.0f, 0.0f));
			Ammo2->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			Ammo2->SetScale(glm::vec3(0.4f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = Ammo2->Add<RenderComponent>();
			renderer->SetMesh(arrowMesh);
			renderer->SetMaterial(arrowMaterial);

			Ammo2->Add<AmmoBehaviour>();
			impo->AddChild(Ammo2);
		}
		GameObject::Sptr door = scene->CreateGameObject("Door");
		{
			// Set position in the scene
			door->SetPostion(glm::vec3(10.0f, -14.8f, 0.0f));
			door->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = door->Add<RenderComponent>();
			renderer->SetMesh(doormesh);
			renderer->SetMaterial(doorMaterial);

			door->Add<DoorBehaviour>();
			impo->AddChild(door);
		}
		GameObject::Sptr trap = scene->CreateGameObject("Traps");
		GameObject::Sptr spike1 = scene->CreateGameObject("Spike Trap");
		{
			// Set position in the scene
			spike1->SetPostion(glm::vec3(11.0f, -11.0f, 0.0f));
			spike1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = spike1->Add<RenderComponent>();
			renderer->SetMesh(SpikeTrap);
			renderer->SetMaterial(spikeMaterial);

			spike1->Add<SpikeBehaviour>();
			trap->AddChild(spike1);
		}
		GameObject::Sptr slime1 = scene->CreateGameObject("Slime Trap");
		{
			// Set position in the scene
			slime1->SetPostion(glm::vec3(8.7f, -11.0f, 0.0f));
			slime1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = slime1->Add<RenderComponent>();
			renderer->SetMesh(slimeTrap);
			renderer->SetMaterial(slimeMaterial);

			slime1->Add<SlimeBehaviour>();
			trap->AddChild(slime1);
		}
		GameObject::Sptr chain1 = scene->CreateGameObject("chain1");
		{
			// Set position in the scene
			chain1->SetPostion(glm::vec3(0.0f, -7.0f, -0.1f));
			chain1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain1->Add<RenderComponent>();
			renderer->SetMesh(ChainModel);
			renderer->SetMaterial(chainMaterial);
			deccor->AddChild(chain1);

		}
		GameObject::Sptr chain12 = scene->CreateGameObject("chain1");
		{
			// Set position in the scene

			chain12->SetPostion(glm::vec3(6.1f, -7.f, -0.1f));
			chain12->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain12->Add<RenderComponent>();
			renderer->SetMesh(ChainModel);
			renderer->SetMaterial(chainMaterial);
			deccor->AddChild(chain12);

		}
		GameObject::Sptr chain2 = scene->CreateGameObject("chain2");
		{
			// Set position in the scene
			chain2->SetPostion(glm::vec3(-6.0f, -7.0f, -0.1f));
			chain2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain2->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);
			deccor->AddChild(chain2);

		}

		GameObject::Sptr chain3 = scene->CreateGameObject("chain3");
		{
			// Set position in the scene
			chain3->SetPostion(glm::vec3(6.1f, -0.61f, -0.1f));
			chain3->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain3->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);
			deccor->AddChild(chain3);

		}
		GameObject::Sptr chain22 = scene->CreateGameObject("chain22");
		{
			// Set position in the scene
			chain22->SetPostion(glm::vec3(4.0f, -7.0f, -0.1f));
			chain22->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = chain22->Add<RenderComponent>();
			renderer->SetMesh(Chain2Model);
			renderer->SetMaterial(chain2Material);

			deccor->AddChild(chain22);
		}
		GameObject::Sptr ribcage = scene->CreateGameObject("ribcage");
		{
			// Set position in the scene
			ribcage->SetPostion(glm::vec3(-6.5f, -5.0f, 0.0f));
			ribcage->SetRotation(glm::vec3(90.0f, 0.0f, 67.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);

			deccor->AddChild(ribcage);
		}
		GameObject::Sptr ribcage2 = scene->CreateGameObject("ribcage2");
		{
			// Set position in the scene
			ribcage2->SetPostion(glm::vec3(6.5f, -2.0f, 0.0f));
			ribcage2->SetRotation(glm::vec3(90.0f, 0.0f, 67.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage2->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);
			deccor->AddChild(ribcage2);

		}
		GameObject::Sptr ribcage3 = scene->CreateGameObject("ribcage3");
		{
			// Set position in the scene
			ribcage3->SetPostion(glm::vec3(6.5f, -6.0f, 0.0f));
			ribcage3->SetRotation(glm::vec3(90.0f, 0.0f, 10.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage3->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);
			deccor->AddChild(ribcage3);

		}
		GameObject::Sptr ribcage4 = scene->CreateGameObject("ribcage4");
		{
			// Set position in the scene
			ribcage4->SetPostion(glm::vec3(6.5f, -10.0f, 0.0f));
			ribcage4->SetRotation(glm::vec3(90.0f, 0.0f, 30.0f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = ribcage4->Add<RenderComponent>();
			renderer->SetMesh(BonesModel);
			renderer->SetMaterial(bonesMaterial);
			deccor->AddChild(ribcage4);
		}
		{//walls start///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			GameObject::Sptr wall = scene->CreateGameObject("Walls");

			GameObject::Sptr wall1 = scene->CreateGameObject("wall 1");
			{
				// Set position in the scene
				wall1->SetPostion(glm::vec3(0.0f, 0.0f, 1.0f));
				wall1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				wall1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall1->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarOne);

				RigidBody::Sptr floorRB = wall1->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall1);
			}
			GameObject::Sptr wall2 = scene->CreateGameObject("wall 2");
			{
				// Set position in the scene
				wall2->SetPostion(glm::vec3(2.5f, -2.5f, 1.0f));
				wall2->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				wall2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall2->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarTwo);

				RigidBody::Sptr floorRB = wall2->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall2);
			}
			GameObject::Sptr wall3 = scene->CreateGameObject("wall 3");
			{
				// Set position in the scene
				wall3->SetPostion(glm::vec3(-2.5f, -2.5f, 1.0f));
				wall3->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				wall3->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall3->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarOne);

				RigidBody::Sptr floorRB = wall3->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall3);
			}
			GameObject::Sptr wall4 = scene->CreateGameObject("wall 4");
			{
				// Set position in the scene
				wall4->SetPostion(glm::vec3(-2.5f, -7.5f, 1.0f));
				wall4->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				wall4->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall4->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarThree);

				RigidBody::Sptr floorRB = wall4->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall4);
			}
			GameObject::Sptr wall5 = scene->CreateGameObject("wall 5");
			{
				// Set position in the scene
				wall5->SetPostion(glm::vec3(-5.f, -10.f, 1.0f));
				wall5->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				wall5->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall5->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFour);

				RigidBody::Sptr floorRB = wall5->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall5);
			}
			GameObject::Sptr wall6 = scene->CreateGameObject("wall 6");
			{
				// Set position in the scene
				wall6->SetPostion(glm::vec3(-7.5f, -12.5f, 1.0f));
				wall6->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				wall6->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall6->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFive);

				RigidBody::Sptr floorRB = wall6->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall6);
			}
			GameObject::Sptr wall7 = scene->CreateGameObject("wall 7");
			{
				// Set position in the scene
				wall7->SetPostion(glm::vec3(-5.f, -15.f, 1.0f));
				wall7->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				wall7->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall7->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarSix);

				RigidBody::Sptr floorRB = wall7->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall7);
			}
			GameObject::Sptr wall8 = scene->CreateGameObject("wall 8");
			{
				// Set position in the scene
				wall8->SetPostion(glm::vec3(-0.f, -15.f, 1.0f));
				wall8->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				wall8->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall8->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarOne);

				RigidBody::Sptr floorRB = wall8->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall8);
			}
			GameObject::Sptr wall9 = scene->CreateGameObject("wall 9");
			{
				// Set position in the scene
				wall9->SetPostion(glm::vec3(2.5f, -12.5f, 1.0f));
				wall9->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				wall9->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall9->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarTwo);

				RigidBody::Sptr floorRB = wall9->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall9);
			}
			GameObject::Sptr wall10 = scene->CreateGameObject("wall 10");
			{
				// Set position in the scene
				wall10->SetPostion(glm::vec3(5.f, -10.f, 1.0f));
				wall10->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				wall10->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall10->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarThree);

				RigidBody::Sptr floorRB = wall10->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall10);
			}
			GameObject::Sptr wall11 = scene->CreateGameObject("wall 11");
			{
				// Set position in the scene
				wall11->SetPostion(glm::vec3(7.5f, -12.5f, 1.0f));
				wall11->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				wall11->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall11->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFour);

				RigidBody::Sptr floorRB = wall11->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall11);
			}
			GameObject::Sptr wall12 = scene->CreateGameObject("wall 12");
			{
				// Set position in the scene
				wall12->SetPostion(glm::vec3(10.f, -15.f, 1.0f));
				wall12->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				wall12->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall12->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFive);

				RigidBody::Sptr floorRB = wall12->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall12);
			}
			GameObject::Sptr wall13 = scene->CreateGameObject("wall 13");
			{
				// Set position in the scene
				wall13->SetPostion(glm::vec3(12.5f, -12.5f, 1.0f));
				wall13->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				wall13->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall13->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarSix);

				RigidBody::Sptr floorRB = wall13->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall13);
			}
			GameObject::Sptr wall14 = scene->CreateGameObject("wall 14");
			{
				// Set position in the scene
				wall14->SetPostion(glm::vec3(12.5f, -7.5f, 1.0f));
				wall14->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				wall14->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall14->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarOne);

				RigidBody::Sptr floorRB = wall14->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall14);
			}
			GameObject::Sptr wall15 = scene->CreateGameObject("wall 15");
			{
				// Set position in the scene
				wall15->SetPostion(glm::vec3(15.f, -5.f, 1.0f));
				wall15->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				wall15->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall15->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarTwo);

				RigidBody::Sptr floorRB = wall15->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall15);
			}
			GameObject::Sptr wall16 = scene->CreateGameObject("wall 16");
			{
				// Set position in the scene
				wall16->SetPostion(glm::vec3(17.5f, -2.5f, 1.0f));
				wall16->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				wall16->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall16->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarThree);

				RigidBody::Sptr floorRB = wall16->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall16);
			}
			GameObject::Sptr wall17 = scene->CreateGameObject("wall 17");
			{
				// Set position in the scene
				wall17->SetPostion(glm::vec3(15.f, 0.f, 1.0f));
				wall17->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				wall17->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall17->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFour);

				RigidBody::Sptr floorRB = wall17->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall17);
			}
			GameObject::Sptr wall18 = scene->CreateGameObject("wall 18");
			{
				// Set position in the scene
				wall18->SetPostion(glm::vec3(10.f, 0.f, 1.0f));
				wall18->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				wall18->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall18->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarFive);

				RigidBody::Sptr floorRB = wall18->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall18);
			}
			GameObject::Sptr wall19 = scene->CreateGameObject("wall 19");
			{
				// Set position in the scene
				wall19->SetPostion(glm::vec3(7.5f, -2.5f, 1.0f));
				wall19->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				wall19->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall19->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarSix);

				RigidBody::Sptr floorRB = wall19->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.3, 2.3, 0.2f)));
				wall->AddChild(wall19);
			}
			GameObject::Sptr wall20 = scene->CreateGameObject("wall 20");
			{
				// Set position in the scene
				wall20->SetPostion(glm::vec3(5.f, -5.f, 1.0f));
				wall20->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				wall20->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = wall20->Add<RenderComponent>();
				renderer->SetMesh(wallMesh);
				renderer->SetMaterial(wallMaterialVarTwo);

				RigidBody::Sptr floorRB = wall20->Add<RigidBody>(RigidBodyType::Static);
				floorRB->AddCollider(BoxCollider::Create(glm::vec3(2.5, 2.5, 0.2f)));
				wall->AddChild(wall20);
			}
		} // walls end

		{//pillars start///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			GameObject::Sptr pillar = scene->CreateGameObject("Pillars");

			GameObject::Sptr pillar1 = scene->CreateGameObject("pillar 1");
			{
				// Set position in the scene
				pillar1->SetPostion(glm::vec3(2.5f, 0.0f, 1.0f));
				pillar1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar1->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar1);
			}
			GameObject::Sptr pillar2 = scene->CreateGameObject("pillar 2");
			{
				// Set position in the scene
				pillar2->SetPostion(glm::vec3(-2.5f, 0.f, 1.0f));
				pillar2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar2->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar2);
			}
			GameObject::Sptr pillar3 = scene->CreateGameObject("pillar 3");
			{
				// Set position in the scene
				pillar3->SetPostion(glm::vec3(-2.5f, -5.f, 1.0f));
				pillar3->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar3->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar3->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar3);
			}
			GameObject::Sptr pillar4 = scene->CreateGameObject("pillar 4");
			{
				// Set position in the scene
				pillar4->SetPostion(glm::vec3(-2.5f, -10.f, 1.0f));
				pillar4->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar4->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar4->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar4);
			}
			GameObject::Sptr pillar5 = scene->CreateGameObject("pillar 5");
			{
				// Set position in the scene
				pillar5->SetPostion(glm::vec3(-7.5f, -10.f, 1.0f));
				pillar5->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar5->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar5->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar5);
			}
			GameObject::Sptr pillar6 = scene->CreateGameObject("pillar 6");
			{
				// Set position in the scene
				pillar6->SetPostion(glm::vec3(-7.5f, -15.f, 1.0f));
				pillar6->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar6->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar6->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar6);
			}
			GameObject::Sptr pillar7 = scene->CreateGameObject("pillar 7");
			{
				// Set position in the scene
				pillar7->SetPostion(glm::vec3(-2.5f, -15.f, 1.0f));
				pillar7->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar7->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar7->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar7);
			}
			GameObject::Sptr pillar8 = scene->CreateGameObject("pillar 8");
			{
				// Set position in the scene
				pillar8->SetPostion(glm::vec3(2.5f, -15.f, 1.0f));
				pillar8->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar8->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar8->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar8);
			}
			GameObject::Sptr pillar9 = scene->CreateGameObject("pillar 9");
			{
				// Set position in the scene
				pillar9->SetPostion(glm::vec3(2.5f, -10.f, 1.0f));
				pillar9->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar9->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar9->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar9);
			}
			GameObject::Sptr pillar10 = scene->CreateGameObject("pillar 10");
			{
				// Set position in the scene
				pillar10->SetPostion(glm::vec3(7.5f, -10.f, 1.0f));
				pillar10->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar10->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar10->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar10);
			}
			GameObject::Sptr pillar11 = scene->CreateGameObject("pillar 11");
			{
				// Set position in the scene
				pillar11->SetPostion(glm::vec3(7.5f, -15.f, 1.0f));
				pillar11->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar11->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
				
				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar11->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar11);
			}
			GameObject::Sptr pillar12 = scene->CreateGameObject("pillar 12");
			{
				// Set position in the scene
				pillar12->SetPostion(glm::vec3(12.5f, -15.f, 1.0f));
				pillar12->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar12->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar12->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar12);
			}
			GameObject::Sptr pillar13 = scene->CreateGameObject("pillar 13");
			{
				// Set position in the scene
				pillar13->SetPostion(glm::vec3(12.5f, -10.f, 1.0f));
				pillar13->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar13->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar13->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar13);
			}
			GameObject::Sptr pillar14 = scene->CreateGameObject("pillar 14");
			{
				// Set position in the scene
				pillar14->SetPostion(glm::vec3(12.5f, -5.f, 1.0f));
				pillar14->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar14->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar14->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar14);
			}
			GameObject::Sptr pillar15 = scene->CreateGameObject("pillar 15");
			{
				// Set position in the scene
				pillar15->SetPostion(glm::vec3(17.5f, -5.f, 1.0f));
				pillar15->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar15->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar15->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar15);
			}
			GameObject::Sptr pillar16 = scene->CreateGameObject("pillar 16");
			{
				// Set position in the scene
				pillar16->SetPostion(glm::vec3(17.5f, 0.f, 1.0f));
				pillar16->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar16->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar16->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar16);
			}
			GameObject::Sptr pillar17 = scene->CreateGameObject("pillar 17");
			{
				// Set position in the scene
				pillar17->SetPostion(glm::vec3(12.5f, 0.f, 1.0f));
				pillar17->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar17->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar17->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar17);
			}
			GameObject::Sptr pillar18 = scene->CreateGameObject("pillar 18");
			{
				// Set position in the scene
				pillar18->SetPostion(glm::vec3(7.5f, 0.f, 1.0f));
				pillar18->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar18->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar18->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar18);
			}
			GameObject::Sptr pillar19 = scene->CreateGameObject("pillar 19");
			{
				// Set position in the scene
				pillar19->SetPostion(glm::vec3(7.5f, -5.f, 1.0f));
				pillar19->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar19->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar19->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar19);
			}
			GameObject::Sptr pillar20 = scene->CreateGameObject("pillar 20");
			{
				// Set position in the scene
				pillar20->SetPostion(glm::vec3(2.5f, -5.f, 1.0f));
				pillar20->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				pillar20->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				// Create and attach a renderer for the modelF
				RenderComponent::Sptr renderer = pillar20->Add<RenderComponent>();
				renderer->SetMesh(pillarMesh);
				renderer->SetMaterial(pillarMaterialVarOne);

				pillar->AddChild(pillar20);
			}
		} // pillar end

		GameObject::Sptr enemies = scene->CreateGameObject("Enemies");
		GameObject::Sptr spider1 = scene->CreateGameObject("Spider");
		{
			// Set position in the scene
			spider1->SetPostion(glm::vec3(-5.0f, -13.0f, 0.0f));
			spider1->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			spider1->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));


			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = spider1->Add<RenderComponent>();
			renderer->SetMesh(spiderMesh);
			renderer->SetMaterial(spiderMaterial);

			spider1->Add<EnemyBehaviourSpider>();
			MorphMeshRenderer::Sptr SpiderWalkAnim = spider1->Add<MorphMeshRenderer>();
			SpiderWalkAnim->SetMorphMeshRenderer(spiderMesh, spiderMaterial);
			Morphanimator::Sptr SpiderWalkAnimator = spider1->Add<Morphanimator>();

			MeshResource::Sptr SpiderWalkAnim1 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000000.obj");
			MeshResource::Sptr SpiderWalkAnim2 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000005.obj");
			MeshResource::Sptr SpiderWalkAnim3 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000010.obj");
			MeshResource::Sptr SpiderWalkAnim4 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000015.obj");
			MeshResource::Sptr SpiderWalkAnim5 = ResourceManager::CreateAsset<MeshResource>("Animations/Spider/SpiderWalk_000020.obj");

			std::vector<MeshResource::Sptr> frames;
			frames.push_back(SpiderWalkAnim1);
			frames.push_back(SpiderWalkAnim2);
			frames.push_back(SpiderWalkAnim3);
			frames.push_back(SpiderWalkAnim4);
			frames.push_back(SpiderWalkAnim5);

			SpiderWalkAnimator->SetInitial();
			SpiderWalkAnimator->SetFrameTime(1.5f);
			SpiderWalkAnimator->SetFrames(frames);

			RigidBody::Sptr spider1RB = spider1->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider1 = BoxCollider::Create(glm::vec3(0.4f, 0.4f, 0.3f));
			collider1->SetPosition(glm::vec3(0.0f, 0.4f, 0.f));
			spider1RB->AddCollider(collider1);
			spider1RB->SetLinearDamping(1.f);

			enemies->AddChild(spider1);

		}
		GameObject::Sptr spider2 = scene->CreateGameObject("spider2");
		{
			// Set position in the scene
			spider2->SetPostion(glm::vec3(-6.0f, -11.0f, 0.0f));
			spider2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spider2->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));


			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = spider2->Add<RenderComponent>();
			renderer->SetMesh(spiderMesh);
			renderer->SetMaterial(spiderMaterial);

			spider2->Add<EnemyBehaviourSpider>();

			RigidBody::Sptr spider2RB = spider2->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider2 = BoxCollider::Create(glm::vec3(0.4f, 0.4f, 0.3f));
			collider2->SetPosition(glm::vec3(0.0f, 0.4f, 0.f));
			spider2RB->AddCollider(collider2);
			spider2RB->SetLinearDamping(1.f);

			enemies->AddChild(spider2);

		}
		GameObject::Sptr golem1 = scene->CreateGameObject("Golem");
		{
			// Set position in the scene
			golem1->SetPostion(glm::vec3(16.0f, -2.5f, 0.0f));
			golem1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			golem1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = golem1->Add<RenderComponent>();
			renderer->SetMesh(golemMesh);
			renderer->SetMaterial(golemMaterial);
			golem1->Add<EnemyPath>();
	
			MorphMeshRenderer::Sptr GolemWalkAnim = golem1->Add<MorphMeshRenderer>();
			GolemWalkAnim->SetMorphMeshRenderer(golemMesh, golemMaterial);
			Morphanimator::Sptr GolemWalkAnimator = golem1->Add<Morphanimator>();
			
			MeshResource::Sptr golemWalkAnim1 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000001.obj");
			MeshResource::Sptr golemWalkAnim2 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000020.obj");
			MeshResource::Sptr golemWalkAnim3 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000040.obj");
			MeshResource::Sptr golemWalkAnim4 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000060.obj");
			MeshResource::Sptr golemWalkAnim5 = ResourceManager::CreateAsset<MeshResource>("Animations/Golem/GolemWalk_000080.obj");

			std::vector<MeshResource::Sptr> frames;
			frames.push_back(golemWalkAnim1);
			frames.push_back(golemWalkAnim2);
			frames.push_back(golemWalkAnim3);
			frames.push_back(golemWalkAnim4);
			frames.push_back(golemWalkAnim5);

			GolemWalkAnimator->SetInitial();
			GolemWalkAnimator->SetFrameTime(1.0f);
			GolemWalkAnimator->SetFrames(frames);

			RigidBody::Sptr golem1RB = golem1->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider3 = BoxCollider::Create(glm::vec3(0.4f, 0.8f, 0.4f));
			collider3->SetPosition(glm::vec3(0.0f, 0.8f, 0.f));
			golem1RB->AddCollider(collider3);
			golem1RB->SetLinearDamping(1.f);

			enemies->AddChild(golem1);
		}

		GameObject::Sptr skeleton1 = scene->CreateGameObject("Skeleton");
		{
			// Set position in the scene
			skeleton1->SetPostion(glm::vec3(10.0f, -13.0f, 0.1f));
			skeleton1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			skeleton1->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));


			// Create and attach a renderer for the model
			RenderComponent::Sptr renderer = skeleton1->Add<RenderComponent>();
			renderer->SetMesh(skeletonMesh);
			renderer->SetMaterial(skeletonMaterial);

			MorphMeshRenderer::Sptr SkeletonWalkAnim = skeleton1->Add<MorphMeshRenderer>();
			SkeletonWalkAnim->SetMorphMeshRenderer(skeletonMesh, skeletonMaterial);
			Morphanimator::Sptr SkeletonWalkAnimator = skeleton1->Add<Morphanimator>();

			MeshResource::Sptr skeletonWalkAnim1 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000001.obj");
			MeshResource::Sptr skeletonWalkAnim2 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000010.obj");
			MeshResource::Sptr skeletonWalkAnim3 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000020.obj");
			MeshResource::Sptr skeletonWalkAnim4 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000030.obj");
			MeshResource::Sptr skeletonWalkAnim5 = ResourceManager::CreateAsset<MeshResource>("Animations/Skeleton/SkeletonWalk_000040.obj");

			std::vector<MeshResource::Sptr> frames;
			frames.push_back(skeletonWalkAnim1);
			frames.push_back(skeletonWalkAnim2);
			frames.push_back(skeletonWalkAnim3);
			frames.push_back(skeletonWalkAnim4);
			frames.push_back(skeletonWalkAnim5);

			
			skeleton1->Add<EnemyBehaviour>();
			RigidBody::Sptr skeletonRB = skeleton1->Add<RigidBody>(RigidBodyType::Dynamic);
			BoxCollider::Sptr collider4 = BoxCollider::Create(glm::vec3(0.35f, 1.2f, 0.35f));
			collider4->SetPosition(glm::vec3(0.0f, 1.2f, 0.f));
			skeletonRB->AddCollider(collider4);
			skeletonRB->SetLinearDamping(1.f);
			enemies->AddChild(skeleton1);

			SkeletonWalkAnimator->SetInitial();
			SkeletonWalkAnimator->SetFrameTime(0.7f);
			SkeletonWalkAnimator->SetFrames(frames);

		}


		GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(1.0f, -7.0f, 2.0f));
			shadowCaster->LookAt(glm::vec3(0.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 100.0f));
		}
		/////////////////////////// UI //////////////////////////////

		GameObject::Sptr UI = scene->CreateGameObject("UI Components");
		GameObject::Sptr MenScreen = scene->CreateGameObject("Menu Screen");
		{
			RectTransform::Sptr transform = MenScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080, });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec3(960, 540, -100));

			GuiPanel::Sptr testPanel = MenScreen->Add<GuiPanel>();
			testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(menuPNG);
			testPanel->SetBorderRadius(1920); //Tinker with 

			MenScreen->Add<MenuScreen>(); UI->AddChild(MenScreen);
		}


		GameObject::Sptr pausScreen = scene->CreateGameObject("Pause Screen");
		{
			RectTransform::Sptr transform = pausScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr testPanel = pausScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(pausePNG);
			testPanel->SetBorderRadius(1920); //Tinker with 
			testPanel->IsEnabled = false;

			pausScreen->Add<PauseScreen>();
			pausScreen->Get<PauseScreen>()->testPanel = testPanel; UI->AddChild(pausScreen);
		}

		GameObject::Sptr WinnerScreen = scene->CreateGameObject("Winner Screen");
		{
			RectTransform::Sptr transform = WinnerScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr testPanel = WinnerScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			testPanel->SetTexture(winPNG);
			testPanel->SetBorderRadius(1920); //Tinker with 
			testPanel->IsEnabled = false;

			WinnerScreen->Add<WinScreen>();
			WinnerScreen->Get<WinScreen>()->testPanel = testPanel; UI->AddChild(WinnerScreen);
		}

		GameObject::Sptr LostScreen = scene->CreateGameObject("Lose Screen");
		{
			RectTransform::Sptr transform = LostScreen->Add<RectTransform>();
			transform->SetMin({ 1920, 1080 });
			transform->SetMax({ 0, 0 });
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr imageLose = LostScreen->Add<GuiPanel>();
			//testPanel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			imageLose->SetTexture(losePNG);
			imageLose->SetBorderRadius(1920); //Tinker with 
			imageLose->IsEnabled = false;

			LostScreen->Add<LoseScreen>();
			LostScreen->Get<LoseScreen>()->imageLose = imageLose; UI->AddChild(LostScreen);

		}

		GameObject::Sptr BandagePack = scene->CreateGameObject("Bandage UI");
		{

			BandagePack->RenderGUI();
			RectTransform::Sptr transform = BandagePack->Add<RectTransform>();

			transform->SetPosition(glm::vec2(220, 920));
			transform->SetSize(glm::vec2(140, 50));

			GuiPanel::Sptr testPanel = BandagePack->Add<GuiPanel>();

			testPanel->SetTexture(Bandage);
			testPanel->SetBorderRadius(0); //Tinker with
			testPanel->IsEnabled = false;


			Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 20.0f);
			font->Bake();

			GuiText::Sptr text = BandagePack->Add<GuiText>();
			text->SetText(std::to_string(bandageCount));
			text->SetFont(font);
			text->SetColor(glm::vec4(0, 1, 0, 0));



			BandagePack->Add<BandageCount>()->BandagePack = testPanel;
			UI->AddChild(BandagePack);
		}

		GameObject::Sptr AmmoPack = scene->CreateGameObject("Ammo UI");
		{

			AmmoPack->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = AmmoPack->Add<RectTransform>();

			transform->SetPosition(glm::vec2(1700, 920));
			transform->SetSize(glm::vec2(99, 50));

			GuiPanel::Sptr testPanel = AmmoPack->Add<GuiPanel>();

			testPanel->SetTexture(ammoHUD);
			testPanel->SetBorderRadius(10); //Tinker with
			testPanel->IsEnabled = false;

			Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 20.0f);
			font->Bake();

			GuiText::Sptr text = AmmoPack->Add<GuiText>();
			text->SetText(std::to_string(ammoCount));
			text->SetFont(font);

			text->SetColor(glm::vec4(0, 1, 0, 0));

			AmmoPack->Add<NormalAmmo>()->AmmoPack = testPanel;
			UI->AddChild(AmmoPack);
		}


		GameObject::Sptr ThreeHealth = scene->CreateGameObject("ThreeHealth");
		{

			ThreeHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = ThreeHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr threeHealth = ThreeHealth->Add<GuiPanel>();
			threeHealth->SetTexture(healthBar);
			threeHealth->SetBorderRadius(52);
			threeHealth->IsEnabled = false;

			ThreeHealth->Add<HealthBehaviour>();
			ThreeHealth->Get<HealthBehaviour>()->threeHealth = threeHealth; UI->AddChild(ThreeHealth);
		}
		GameObject::Sptr TwoHealth = scene->CreateGameObject("TwoHealth");
		{

			TwoHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = TwoHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr twoHealth = TwoHealth->Add<GuiPanel>();
			twoHealth->SetTexture(healthBar1);
			twoHealth->SetBorderRadius(52);
			twoHealth->IsEnabled = false;


			TwoHealth->Add<HealthBehaviour2>();
			TwoHealth->Get<HealthBehaviour2>()->twoHealth = twoHealth; UI->AddChild(TwoHealth);
		}
		GameObject::Sptr OneHealth = scene->CreateGameObject("OneHealth");
		{

			OneHealth->RenderGUI();
			//glEnable(GL_BLEND);
			RectTransform::Sptr transform = OneHealth->Add<RectTransform>();

			transform->SetPosition(glm::vec2(260, 90));
			transform->SetSize(glm::vec2(130, 0));

			GuiPanel::Sptr oneHealth = OneHealth->Add<GuiPanel>();
			oneHealth->SetTexture(healthBar2);
			oneHealth->SetBorderRadius(52);
			oneHealth->IsEnabled = false;

			OneHealth->Add<HealthBehaviour1>();
			OneHealth->Get<HealthBehaviour1>()->oneHealth = oneHealth; UI->AddChild(OneHealth);
		}

		GameObject::Sptr CrossHair = scene->CreateGameObject("Crosshair");
		{
			RectTransform::Sptr transform = CrossHair->Add<RectTransform>();
			transform->SetMin({ 50 ,50 });
			transform->SetMax({ 50, 50 });
			transform->SetSize(glm::vec2(5, 5));
			transform->SetPosition(glm::vec2(960, 540));

			GuiPanel::Sptr CrossHair2 = CrossHair->Add<GuiPanel>();

			CrossHair2->SetColor(glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
			CrossHair2->SetTexture(Crosshair);
			CrossHair2->SetBorderRadius(19); //Tinker with 
			UI->AddChild(CrossHair);
		}

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
