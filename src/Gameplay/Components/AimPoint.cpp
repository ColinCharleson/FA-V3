#include "Gameplay/Components/AimPoint.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/SimpleCameraControl.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

void AimPoint::Awake()
{
}

void AimPoint::RenderImGui()
{
}

nlohmann::json AimPoint::ToJson() const
{
	return {

	};
}

AimPoint::AimPoint() :
	IComponent(),
	_impulse(10.0f)
{
}

AimPoint::~AimPoint() = default;

AimPoint::Sptr AimPoint::FromJson(const nlohmann::json & blob)
{
	AimPoint::Sptr result = std::make_shared<AimPoint>();

	return result;
}
extern int ammoCount;
extern float playerX, playerY;
extern int playerHealth;
extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern glm::quat currentRot;

extern bool onMenu;
extern bool canShoot;
extern bool gamePaused;
extern glm::vec3 forward;
void AimPoint::Update(float deltaTime)
{
	if (gamePaused == false)
	{
		if (playerHealth > 0)
		{
			if (arrowOut == false)
			{
				GetGameObject()->SetScale(glm::vec3(0.3f));
			}
			else if (arrowOut == true)
			{

				GetGameObject()->SetScale(glm::vec3(0.01f));
			}
			 
			if (ammoCount == 0)
			{
				GetGameObject()->SetScale(glm::vec3(0.01f));
			}
		}
	}
}

