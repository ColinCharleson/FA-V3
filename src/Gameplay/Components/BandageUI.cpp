#include "Gameplay/Components/BandageUI.h"
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
void BandageUI::Awake()
{
}

void BandageUI::RenderImGui() {
}

nlohmann::json BandageUI::ToJson() const {
	return {
		
	};
}

BandageUI::BandageUI() :
	IComponent(),
	_impulse(10.0f)
{ }

BandageUI::~BandageUI() = default;

BandageUI::Sptr BandageUI::FromJson(const nlohmann::json& blob) {
	BandageUI::Sptr result = std::make_shared<BandageUI>();

	return result;
}

extern int bandageCount;

extern bool onMenu;
extern bool onUIBandage;
void BandageUI::Update(float deltaTime) 
{

	Gameplay::IComponent::Sptr ptr = Bandageui.lock();
	if (onUIBandage == true)
	{

		if (onMenu == false)
		{
			ptr->IsEnabled = true;


		}




	}
}

