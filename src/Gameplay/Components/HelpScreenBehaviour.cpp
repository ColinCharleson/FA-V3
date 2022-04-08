#include "Gameplay/Components/HelpScreenBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/InputEngine.h"

void HelpScreen::Awake()
{
}

void HelpScreen::RenderImGui() {
}

nlohmann::json HelpScreen::ToJson() const {
	return {
		
	};
}

HelpScreen::HelpScreen() :
	IComponent(),
	_impulse(10.0f)
{ }

HelpScreen::~HelpScreen() = default;

HelpScreen::Sptr HelpScreen::FromJson(const nlohmann::json& blob) {
	HelpScreen::Sptr result = std::make_shared<HelpScreen>();

	return result;
}

extern bool gamePaused, onMenu;
bool helping = false;
bool pressed = false;
void HelpScreen::Update(float deltaTime) {
	Gameplay::IComponent::Sptr ptr = testPanel.lock();
	
	if (onMenu == true)
	{
		if (InputEngine::IsKeyDown(GLFW_KEY_H))
		{
			if (pressed == false)
			{
				helping = !helping;
				pressed = true;
			}
		}
		if (InputEngine::GetKeyState(GLFW_KEY_H) == ButtonState::Up)
		{
			pressed = false;
		}
		if (helping == false)
		{
			ptr->IsEnabled = false;
		}
		else
		{
			ptr->IsEnabled = true;
		}
	}
	else
	{
		if (gamePaused == false)
		{
			ptr->IsEnabled = false;
			helping = false;
		}
	}

}

