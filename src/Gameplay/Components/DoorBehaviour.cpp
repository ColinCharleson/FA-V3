#include "Gameplay/Components/DoorBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

void DoorBehaviour::Awake()
{
}

void DoorBehaviour::RenderImGui() {
}

nlohmann::json DoorBehaviour::ToJson() const {
	return {
		
	};
}

DoorBehaviour::DoorBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

DoorBehaviour::~DoorBehaviour() = default;

DoorBehaviour::Sptr DoorBehaviour::FromJson(const nlohmann::json& blob) {
	DoorBehaviour::Sptr result = std::make_shared<DoorBehaviour>();

	return result;
}
extern float playerX, playerY;
extern bool hasKey, levelComplete;
extern int roomType, progressScore;

void DoorBehaviour::Update(float deltaTime) {

	bool pressed = InputEngine::IsKeyDown(GLFW_KEY_E);
	
	if (pressed) 
	{
		if (_isPressed == false) 
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y -  playerY, 2) * 2)) <= 2)
			{
				if (hasKey == true)
				{
					levelComplete = true;
				}
				else
				{
				}
			}
				
		}
		_isPressed = pressed;
	} 
	else 
	{
		_isPressed = false;
	}
}

