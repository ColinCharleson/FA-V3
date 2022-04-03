#include "Gameplay/Components/MenuScreenBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void MenuScreen::Awake()
{
}

void MenuScreen::RenderImGui() {
}

nlohmann::json MenuScreen::ToJson() const {
	return {
		
	};
}

MenuScreen::MenuScreen() :
	IComponent(),
	_impulse(10.0f)
{ }

MenuScreen::~MenuScreen() = default;

MenuScreen::Sptr MenuScreen::FromJson(const nlohmann::json& blob) {
	MenuScreen::Sptr result = std::make_shared<MenuScreen>();

	return result;
}

extern bool onMenu;
void MenuScreen::Update(float deltaTime) {
	if (onMenu == false)
	{
		GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
	}
}

