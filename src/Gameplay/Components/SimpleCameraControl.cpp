#include "Gameplay/Components/SimpleCameraControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"
#include <math.h>
SimpleCameraControl::SimpleCameraControl() :
	IComponent(),
	_mouseSensitivity({ 0.5f, 0.3f }),
	_moveSpeeds(glm::vec3(1.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f)),
	_isMousePressed(false)
{
}

SimpleCameraControl::~SimpleCameraControl() = default;

extern bool slimeSlow;
extern bool gamePaused;
extern int ammoCount, playerHealth, bandageCount;
extern glm::quat currentRot;
extern bool startPlaying;

void SimpleCameraControl::Update(float deltaTime)
{
	if (gamePaused == false)
	{

		if (InputEngine::IsKeyDown(GLFW_KEY_SPACE))
			startPlaying = true;

		if (startPlaying == true)
		{

			glm::dvec2 currentMousePos = InputEngine::GetMousePos();

			_currentRot.x += static_cast<float>(currentMousePos.x - _prevMousePos.x) * _mouseSensitivity.x;
			_currentRot.y += static_cast<float>(currentMousePos.y - _prevMousePos.y)* _mouseSensitivity.y;
			glm::quat rotX = glm::angleAxis(glm::radians(-_currentRot.x), glm::vec3(0, 0, 1));
			glm::quat rotY = glm::angleAxis(glm::radians(-_currentRot.y), glm::vec3(1, 0, 0));
			currentRot = rotX * rotY;
			GetGameObject()->SetRotation(currentRot);

			_prevMousePos = currentMousePos;

			glm::vec3 input = glm::vec3(0.0f);

			if (InputEngine::IsKeyDown(GLFW_KEY_H))
			{
				if (bandageCount > 0)
					if (playerHealth < 3)
					{
						playerHealth += 1;
						bandageCount -= 1;
						std::cout << "Player Health: " << playerHealth << std::endl;
						std::cout << "Bandage Count: " << bandageCount << std::endl;
					}
			}

			if (slimeSlow == false)
			{
				if (InputEngine::IsKeyDown(GLFW_KEY_W))
				{
					input.z -= _moveSpeeds.x;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_S))
				{
					input.z += _moveSpeeds.x;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_A))
				{
					input.x -= _moveSpeeds.y;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_D))
				{
					input.x += _moveSpeeds.y;
				}
			}
			else
			{
				if (InputEngine::IsKeyDown(GLFW_KEY_W))
				{
					input.z -= _moveSpeeds.x / 4.0f;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_S))
				{
					input.z += _moveSpeeds.x / 4.0f;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_A))
				{
					input.x -= _moveSpeeds.y / 4.0f;
				}
				if (InputEngine::IsKeyDown(GLFW_KEY_D))
				{
					input.x += _moveSpeeds.y / 4.0f;
				}
			}

			input *= deltaTime;

			glm::vec3 worldMovement = currentRot * glm::vec4(input, 1.0f);
			worldMovement.z = 0;
			GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldMovement);
			
			if (_currentRot.y > 0)
				_currentRot.y = 0;

			if (_currentRot.y < -160)
				_currentRot.y = -160;
		}

		_prevMousePos = InputEngine::GetMousePos();
	}
	else
	{
		_prevMousePos = glm::vec2(0,0);
		_currentRot = glm::vec2(0, 0);
	}
}

void SimpleCameraControl::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat2, "Mouse Sensitivity", &_mouseSensitivity.x, 0.01f);
	LABEL_LEFT(ImGui::DragFloat3, "Move Speed       ", &_moveSpeeds.x, 0.01f, 0.01f);
	LABEL_LEFT(ImGui::DragFloat, "Shift Multiplier ", &_shiftMultipler, 0.01f, 1.0f);
}

nlohmann::json SimpleCameraControl::ToJson() const
{
	return {
		{ "mouse_sensitivity", _mouseSensitivity },
		{ "move_speed", _moveSpeeds },
		{ "shift_mult", _shiftMultipler }
	};
}

SimpleCameraControl::Sptr SimpleCameraControl::FromJson(const nlohmann::json & blob)
{
	SimpleCameraControl::Sptr result = std::make_shared<SimpleCameraControl>();
	result->_mouseSensitivity = JsonGet(blob, "mouse_sensitivity", result->_mouseSensitivity);
	result->_moveSpeeds = JsonGet(blob, "move_speed", result->_moveSpeeds);
	result->_shiftMultipler = JsonGet(blob, "shift_mult", 2.0f);
	return result;
}
