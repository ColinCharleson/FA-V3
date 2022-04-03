#include "Gameplay/Components/ItemAmmoBehaviour.h"
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

void AmmoBehaviour::Awake()
{
}

void AmmoBehaviour::RenderImGui() {
}

nlohmann::json AmmoBehaviour::ToJson() const {
	return {
		
	};
}

AmmoBehaviour::AmmoBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

AmmoBehaviour::~AmmoBehaviour() = default;

AmmoBehaviour::Sptr AmmoBehaviour::FromJson(const nlohmann::json& blob) {
	AmmoBehaviour::Sptr result = std::make_shared<AmmoBehaviour>();

	return result;
}
extern float playerX, playerY;
extern bool hasKey;
extern int ammoCount, playerHealth, bandageCount;
extern bool ammoOn;
void AmmoBehaviour::Update(float deltaTime) {
	if (InputEngine::IsKeyDown(GLFW_KEY_E))
	{
		if (_isPressed == false) 
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 2)
			{
				GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
					ammoCount += 3;
					std::cout << "Ammo count: " << ammoCount << std::endl;
			}
		}
	} 
}

