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

#include "Gameplay/Physics/EnemyPath.h"
#include "NOU/Mesh.h"
#include "NOU/CCamera.h"






void EnemyPath::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json EnemyPath::ToJson() const
{
	return {
		{ "impulse", _impulse }
	};
}

EnemyPath::EnemyPath() :
	IComponent(),
	_impulse(10.0f)
{
}


EnemyPath::Sptr EnemyPath::FromJson(const nlohmann::json& blob)
{
	EnemyPath::Sptr result = std::make_shared<EnemyPath>();
	result->_impulse = blob["impulse"];
	return result;
}

extern float playerX, playerY;
extern int ammoCount, playerHealth, bandageCount;

EnemyPath::~EnemyPath() = default;



template<typename T>
T Lerp(const T & a, const T & b, float t)
{


	return (1.0f - t) * a + t * b;


}
extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern int golemHealth , spiderHealth , skeletonHealth ;

extern float deathTime ;
extern bool canShoot;

extern bool gamePaused;


bool isAlive = true;
void EnemyPath::Update(float deltaTime)
{
	if (gamePaused == false)
	{
		if ((sqrt(pow(GetGameObject()->GetPosition().x - boltX, 2) + pow(GetGameObject()->GetPosition().y - boltY, 2) + pow(GetGameObject()->GetPosition().z - boltZ, 2) * 2)) <= 1.0f)
		{
			if (arrowOut == true)
				isAlive = false;
		}

		if (isAlive == true)
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 4)
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
				std::vector<glm::vec3> _points = { glm::vec3(16.0f, -2.5f, 0.0f), glm::vec3(9.0f, -4.0f, 0.0f), };

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
				glm::vec3 b = _points[((m_segmentIndex)+_points.size()) % _points.size()];

				GetGameObject()->SetPostion(Lerp(a, b, t));
				GetGameObject()->LookAt(glm::vec3(b.x, b.y, 0));
			}
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 1.5)
			{

				if (deathTime <= 0)
				{
					playerHealth -= 1;
					deathTime = 2;
					std::cout << "Player health: " << playerHealth << std::endl;
					canShoot = false;

					if (playerHealth > 0)
					{
						playerHealth -= 1;
						std::cout << "Player health: " << playerHealth << std::endl;

					}
				}

			}
			if (deathTime <= 0)
			{
				canShoot = true;
			}
			if (deathTime > 0)
			{
				deathTime -= 1 * deltaTime;
			}


		}
		else
		{
			GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());





		}
	}
}



