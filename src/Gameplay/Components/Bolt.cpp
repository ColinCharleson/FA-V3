#include "Gameplay/Components/Bolt.h"
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

void Bolt::Awake()
{
}

void Bolt::RenderImGui()
{
}

nlohmann::json Bolt::ToJson() const
{
	return {

	};
}

Bolt::Bolt() :
	IComponent(),
	_impulse(10.0f)
{
}

Bolt::~Bolt() = default;

Bolt::Sptr Bolt::FromJson(const nlohmann::json & blob)
{
	Bolt::Sptr result = std::make_shared<Bolt>();

	return result;
}
extern int ammoCount;
extern float playerX, playerY;
extern int playerHealth;
extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern glm::quat currentRot;
float fireTime = 0;
glm::quat firedRot;

extern bool onMenu;
extern bool canShoot;
extern bool gamePaused;
extern glm::vec3 forward;

void Bolt::Update(float deltaTime)
{
	if (gamePaused == false)
	{
		boltX = GetGameObject()->GetPosition().x;
		boltY = GetGameObject()->GetPosition().y;
		boltZ = GetGameObject()->GetPosition().z;
		if (playerHealth > 0)
		{
			if (InputEngine::IsMouseButtonDown(0))

				if (ammoCount > 0)
				{
					if (canShoot == true)
					{
						if (arrowOut == false)
						{
							arrowOut = true;
							firedRot = currentRot;
							fireTime = 3;
						}
					}
				} 

			if (arrowOut == false)
			{
				GetGameObject()->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));
				GetGameObject()->SetPostion(glm::vec3(playerX, playerY, 0.6f));
				GetGameObject()->SetRotation(currentRot);

			}
			else if (arrowOut == true)
			{

				glm::vec3 worldMovement = firedRot * glm::vec3(0, 0, -1);
				worldMovement *= deltaTime * 20;

				GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldMovement);

				GetGameObject()->SetScale(glm::vec3(0.3f));
			}

			if (fireTime > 0)
			{
				fireTime -= 1 * deltaTime;
			}
			else
			{
				if (arrowOut == true)
					ammoCount -= 1;

				arrowOut = false;
			}
		}
	}
}

