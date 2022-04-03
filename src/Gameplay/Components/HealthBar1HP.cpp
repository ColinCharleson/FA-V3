#include "Gameplay/Components/HealthBar1HP.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void HealthBehaviour1::Awake()
{
}

void HealthBehaviour1::RenderImGui() {
}

nlohmann::json HealthBehaviour1::ToJson() const {
	return {
		
	};
}

HealthBehaviour1::HealthBehaviour1() :
	IComponent(),
	_impulse(10.0f)
{ }

HealthBehaviour1::~HealthBehaviour1() = default;

HealthBehaviour1::Sptr HealthBehaviour1::FromJson(const nlohmann::json& blob) {
	HealthBehaviour1::Sptr result = std::make_shared<HealthBehaviour1>();

	return result;
}

extern int playerHealth;
void HealthBehaviour1::Update(float deltaTime) {
	Gameplay::IComponent::Sptr ptr = oneHealth.lock();
	if (playerHealth == 1)
	{
		ptr->IsEnabled = true;
	}
	else
	{
		ptr->IsEnabled = false;
	}
}

