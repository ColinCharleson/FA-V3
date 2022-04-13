#include "Gameplay/Components/PauseScreenBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void PauseScreen::Awake()
{
}

void PauseScreen::RenderImGui() {
}

nlohmann::json PauseScreen::ToJson() const {
	return {
		
	};
}

PauseScreen::PauseScreen() :
	IComponent(),
	_impulse(10.0f)
{ }

PauseScreen::~PauseScreen() = default;

PauseScreen::Sptr PauseScreen::FromJson(const nlohmann::json& blob) {
	PauseScreen::Sptr result = std::make_shared<PauseScreen>();

	return result;
}

extern bool gamePaused;
extern bool onMenu;
extern bool onUiHealth;
extern bool onUIBandage;
extern bool onUiAmmo;

void PauseScreen::Update(float deltaTime) {
	Gameplay::IComponent::Sptr ptr = testPanel.lock();

	
	

		if (gamePaused == true)
		{
			if (onMenu == false)
			{
				ptr->IsEnabled = true;
				
			}
			
		}
		else
		{
			if (onMenu == false)
			{
				ptr->IsEnabled = false;

			}
			
		}
	

}

