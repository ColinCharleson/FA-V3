#include "Gameplay/Components/HealthBar2HP.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void HealthBehaviour2::Awake()
{
}

void HealthBehaviour2::RenderImGui() {
}

nlohmann::json HealthBehaviour2::ToJson() const {
	return {
		
	};
}

HealthBehaviour2::HealthBehaviour2() :
	IComponent(),
	_impulse(10.0f)
{ }

HealthBehaviour2::~HealthBehaviour2() = default;

HealthBehaviour2::Sptr HealthBehaviour2::FromJson(const nlohmann::json& blob) {
	HealthBehaviour2::Sptr result = std::make_shared<HealthBehaviour2>();

	return result;
}

extern int playerHealth;
void HealthBehaviour2::Update(float deltaTime) {
	Gameplay::IComponent::Sptr ptr = twoHealth.lock();
	if (playerHealth == 2)
	{
		ptr->IsEnabled = true;
	}
	else
	{
		ptr->IsEnabled = false;
	}
}

