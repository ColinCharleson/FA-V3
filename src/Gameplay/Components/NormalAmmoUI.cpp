#include "Gameplay/Components/NormalAmmoUI.h"
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
void NormalAmmoUI::Awake()
{
}

void NormalAmmoUI::RenderImGui() {
}

nlohmann::json NormalAmmoUI::ToJson() const {
	return {
		
	};
}

NormalAmmoUI::NormalAmmoUI() :
	IComponent(),
	_impulse(10.0f)
{ }

NormalAmmoUI::~NormalAmmoUI() = default;

NormalAmmoUI::Sptr NormalAmmoUI::FromJson(const nlohmann::json& blob) {
	NormalAmmoUI::Sptr result = std::make_shared<NormalAmmoUI>();

	return result;
}

extern int ammoCount;
extern bool onMenu;
extern bool onUiAmmo;
void NormalAmmoUI::Update(float deltaTime) {

	Gameplay::IComponent::Sptr ptr = AmmoPackUI.lock();
	if (onUiAmmo == true)
	{

		if (onMenu == false)
		{
			ptr->IsEnabled = true;


		}

	
	}

		

		

	
}

