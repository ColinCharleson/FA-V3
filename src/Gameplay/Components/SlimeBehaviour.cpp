#include "Gameplay/Components/SlimeBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void SlimeBehaviour::Awake()
{
}

void SlimeBehaviour::RenderImGui() {
}

nlohmann::json SlimeBehaviour::ToJson() const {
	return {
		
	};
}

SlimeBehaviour::SlimeBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

SlimeBehaviour::~SlimeBehaviour() = default;

SlimeBehaviour::Sptr SlimeBehaviour::FromJson(const nlohmann::json& blob) {
	SlimeBehaviour::Sptr result = std::make_shared<SlimeBehaviour>();

	return result;
}
extern float playerX, playerY;
extern bool slimeSlow;
void SlimeBehaviour::Update(float deltaTime) {
	if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 1)
	{
		slimeSlow = true;
	}
	else
	{
		slimeSlow = false;
	}
}

