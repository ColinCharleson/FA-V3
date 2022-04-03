#include "Gameplay/Components/SpikeBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void SpikeBehaviour::Awake()
{
}

void SpikeBehaviour::RenderImGui() {
}

nlohmann::json SpikeBehaviour::ToJson() const {
	return {
		
	};
}

SpikeBehaviour::SpikeBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

SpikeBehaviour::~SpikeBehaviour() = default;

SpikeBehaviour::Sptr SpikeBehaviour::FromJson(const nlohmann::json& blob) {
	SpikeBehaviour::Sptr result = std::make_shared<SpikeBehaviour>();

	return result;
}
extern float playerX, playerY;
extern int ammoCount, playerHealth, bandageCount;

void SpikeBehaviour::Update(float deltaTime) {
	if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 1.2f)
	{
		playerHealth = 0;
	}
}

