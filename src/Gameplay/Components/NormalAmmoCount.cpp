#include "Gameplay/Components/NormalAmmoCount.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/GameObject.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
void NormalAmmo::Awake()
{
}

void NormalAmmo::RenderImGui() {
}

nlohmann::json NormalAmmo::ToJson() const {
	return {
		
	};
}

NormalAmmo::NormalAmmo() :
	IComponent(),
	_impulse(10.0f)
{ }

NormalAmmo::~NormalAmmo() = default;

NormalAmmo::Sptr NormalAmmo::FromJson(const nlohmann::json& blob) {
	NormalAmmo::Sptr result = std::make_shared<NormalAmmo>();

	return result;
}

extern int ammoCount;
extern bool onMenu;
extern bool onUiAmmo;
void NormalAmmo::Update(float deltaTime) {

	Gameplay::IComponent::Sptr ptr = AmmoPack.lock();
	if (onUiAmmo == true)
	

		if (onMenu == false)
		{
			ptr->IsEnabled = true;
		}

		if (ammoCount == 11)
			GetGameObject()->Get<GuiText>()->SetText("11");
		if (ammoCount == 10)
			GetGameObject()->Get<GuiText>()->SetText("10");
		if (ammoCount == 9)
			GetGameObject()->Get<GuiText>()->SetText("9");
		if (ammoCount == 8)
			GetGameObject()->Get<GuiText>()->SetText("8");
		if (ammoCount == 7)
			GetGameObject()->Get<GuiText>()->SetText("7");
		if (ammoCount == 6)
			GetGameObject()->Get<GuiText>()->SetText("6");
		if (ammoCount == 5)
			GetGameObject()->Get<GuiText>()->SetText("5");
		if (ammoCount == 4)
			GetGameObject()->Get<GuiText>()->SetText("4");
		if (ammoCount == 3)
			GetGameObject()->Get<GuiText>()->SetText("3");
		if (ammoCount == 2)
			GetGameObject()->Get<GuiText>()->SetText("2");
		if (ammoCount == 1)
			GetGameObject()->Get<GuiText>()->SetText("1");
		if (ammoCount == 0)
			GetGameObject()->Get<GuiText>()->SetText("0");

	
}

