#include "Gameplay/Physics/TriggerVolume.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "Utils/GlmBulletConversions.h"

#include "Gameplay/Components/EnemyAi.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"

#include "Gameplay/Physics/EnemyPathBezeir.h"
#include "NOU/Mesh.h"
#include "NOU/CCamera.h"






	void EnemyPathBezeir::RenderImGui() {
		LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
	}

	nlohmann::json EnemyPathBezeir::ToJson() const {
		return {
			{ "impulse", _impulse }
		};
	}

	EnemyPathBezeir::EnemyPathBezeir() :
		IComponent(),
		_impulse(10.0f)
	{ }


	EnemyPathBezeir::Sptr EnemyPathBezeir::FromJson(const nlohmann::json& blob) {
		EnemyPathBezeir::Sptr result = std::make_shared<EnemyPathBezeir>();
		result->_impulse = blob["impulse"];
		return result;
	}

	extern float playerX, playerY;
	extern int ammoCount, playerHealth, bandageCount;

	EnemyPathBezeir::~EnemyPathBezeir() = default;



	template<typename T>
	T Bezeir(const T& a, const T& b, const T& c, const T& d, float t)
	{
	
	
		return ((-a + 3.f * b - 3.f * c + d) * t * t * t +
			(3.f * a - 6.f * b + 3.f * c) * t * t +
			(3.f * -a + 3.f * b) * t +
			(a));
	
		
	}
	extern float boltX, boltY, boltZ;
	extern bool arrowOut;

	
	void EnemyPathBezeir::Update(float deltaTime)
	{
		

	
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 2)
			{
				if (GetGameObject()->GetPosition().x > playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x - 0.01, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y > playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y - 0.01, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().x < playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x + 0.01, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y < playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y + 0.01, GetGameObject()->GetPosition().z));
				}

				GetGameObject()->LookAt(glm::vec3(playerX, playerY, 0));
			}
			else
			{
				std::vector<glm::vec3> _points = { glm::vec3(6.0f, -11.0f, 0.0f), glm::vec3(2.0f, -11.0f, 0.0f), glm::vec3(2.0f, -3.0f, 0.0f), glm::vec3(6.0f, -3.0f, 0.0f) };

				_timer += deltaTime;

				while (_timer >= m_segmentTravelTime)
				{
					_timer -= m_segmentTravelTime;

					m_segmentIndex += 1;

					if (m_segmentIndex == _points.size())
					{
						m_segmentIndex = 0;
					}
				}

				float t = _timer / m_segmentTravelTime;

				glm::vec3 a = _points[((m_segmentIndex -1) +_points.size()) % _points.size()];
				glm::vec3 b = _points[((m_segmentIndex )     +_points.size()) % _points.size()];
				glm::vec3 c = _points[((m_segmentIndex + 1) +_points.size()) % _points.size()];;
		      	glm::vec3 d = _points[((m_segmentIndex + 2) +_points.size()) % _points.size()];;

			

			GetGameObject()->SetPostion(Bezeir(a, b,c,d, t));
			
			

				if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 0.5)
				{
					if (playerHealth > 0)
					{
						playerHealth -= 1;
						std::cout << "Player health: " << playerHealth << std::endl;
					}
				}
			}
		
	}




