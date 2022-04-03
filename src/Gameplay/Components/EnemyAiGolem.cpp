#include "Gameplay/Components/EnemyAiGolem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
using namespace std;
void EnemyBehaviourGolem::Awake()
{

	_body = GetComponent<Gameplay::Physics::RigidBody>();

	if (_body == nullptr) {
		_playerInTrigger = false;
		_startPosiiton = GetGameObject()->GetPosition();
	}
}


void EnemyBehaviourGolem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json EnemyBehaviourGolem::ToJson() const {
	return {
		{ "impulse", _impulse }
	};
}

EnemyBehaviourGolem::EnemyBehaviourGolem() :
	IComponent(),
	_impulse(10.0f)
{ }


EnemyBehaviourGolem::Sptr EnemyBehaviourGolem::FromJson(const nlohmann::json& blob) {
	EnemyBehaviourGolem::Sptr result = std::make_shared<EnemyBehaviourGolem>();
	result->_impulse = blob["impulse"];
	return result;
}

extern float playerX, playerY;
extern int ammoCount, playerHealth, bandageCount;

extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern bool canShoot;
extern bool gamePaused;
 int golemHealth = 3;
 float dmgTime2 = 0;

EnemyBehaviourGolem::~EnemyBehaviourGolem() = default;

float deathTime1 = 0;

void EnemyBehaviourGolem::Update(float deltaTime)
{
	

	if (gamePaused == false)
	{
		if ((sqrt(pow(GetGameObject()->GetPosition().x - boltX, 2) + pow(GetGameObject()->GetPosition().y - boltY, 2) + pow(GetGameObject()->GetPosition().z - boltZ, 2) * 2)) <= 1.0f)
		{
			if (arrowOut == true)
			{
				golemHealth -= 1;
				if (dmgTime2 <= 0)
				{
					golemHealth -= 1;
					cout << golemHealth;
					dmgTime2 = 2;

					if (golemHealth == 0)
					{
						GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
					}
				}
			}
		}
		if (dmgTime2 >= 0)
		{
			dmgTime2 -= 1 * deltaTime;
		}
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

		if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 1.5)
		{
			if (playerHealth > 0)
			{
				if (deathTime1 <= 0)
				{
					playerHealth -= 1;
					deathTime1 = 2;
					std::cout << "Player health: " << playerHealth << std::endl;
					canShoot = false;
				}
			}

		}


		if (deathTime1 <= 0)
		{

			canShoot = true;
		}
		if (deathTime1 > 0)
		{
			deathTime1 -= 1 * deltaTime;
		}
	}
}

