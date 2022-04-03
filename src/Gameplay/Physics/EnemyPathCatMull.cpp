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

#include "Gameplay/Physics/EnemyPathCatMull.h"
#include "NOU/Mesh.h"
#include "NOU/CCamera.h"






	void EnemyPathCatMull::RenderImGui() {
		LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
	}

	nlohmann::json EnemyPathCatMull::ToJson() const {
		return {
			{ "impulse", _impulse }
		};
	}

	EnemyPathCatMull::EnemyPathCatMull() :
		IComponent(),
		_impulse(10.0f)
	{ }


	EnemyPathCatMull::Sptr EnemyPathCatMull::FromJson(const nlohmann::json& blob) {
		EnemyPathCatMull::Sptr result = std::make_shared<EnemyPathCatMull>();
		result->_impulse = blob["impulse"];
		return result;
	}

	extern float playerX, playerY;
	extern int ammoCount, playerHealth, bandageCount;

	EnemyPathCatMull::~EnemyPathCatMull() = default;



	template<typename T>
	T Catmull(const T& a, const T& b, const T& c, const T& d, float t)
	{
	
	
		return  0.5f * ((-a + 3.f * b - 3.f * c + d) * t * t * t +
			(2.f * a - 5.f * b + 4.f * c + -d) * t * t +
			(-a + b) * t +
			(2.f * b));
	
		
	}
	extern float boltX, boltY, boltZ;
	extern bool arrowOut;

	
	void EnemyPathCatMull::Update(float deltaTime)
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
				std::vector<glm::vec3> _points = { glm::vec3(6.0f, -11.0f, 0.0f), glm::vec3(5.0f, -11.0f, 0.0f),glm::vec3(5.0f, -7.0f, 0.0f), glm::vec3(6.0f, -7.0f, 0.0f) };

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


				glm::vec3 a = _points[((m_segmentIndex - 1) + _points.size()) % _points.size()];
				glm::vec3 b = _points[((m_segmentIndex)     +_points.size()) % _points.size()];
				glm::vec3 c = _points[((m_segmentIndex + 1) + _points.size()) % _points.size()];;
				glm::vec3 d = _points[((m_segmentIndex + 2) + _points.size()) % _points.size()];;

			

			GetGameObject()->SetPostion(Catmull(a, b, c, d, t));
			GetGameObject()->LookAt(glm::vec3(b.x, b.y, 0));
			

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




