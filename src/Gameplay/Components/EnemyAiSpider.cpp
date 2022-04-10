#include "Gameplay/Components/EnemyAiSpider.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
using namespace std;
void EnemyBehaviourSpider::Awake()
{

	_body = GetComponent<Gameplay::Physics::RigidBody>();

	if (_body == nullptr) {
		_playerInTrigger = false;
		_startPosiiton = GetGameObject()->GetPosition();
	}
}


void EnemyBehaviourSpider::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json EnemyBehaviourSpider::ToJson() const {
	return {
		{ "impulse", _impulse }
	};
}

EnemyBehaviourSpider::EnemyBehaviourSpider() :
	IComponent(),
	_impulse(10.0f)
{ }


EnemyBehaviourSpider::Sptr EnemyBehaviourSpider::FromJson(const nlohmann::json& blob) {
	EnemyBehaviourSpider::Sptr result = std::make_shared<EnemyBehaviourSpider>();
	result->_impulse = blob["impulse"];
	return result;
}

extern float playerX, playerY;
extern int ammoCount, playerHealth, bandageCount;
extern bool gameWin;
extern float boltX, boltY, boltZ;
extern bool arrowOut;
extern bool canShoot;
extern bool gamePaused;


EnemyBehaviourSpider::~EnemyBehaviourSpider() = default;

float deathTime2 = 0;

void EnemyBehaviourSpider::Update(float deltaTime)
{
	
	if (gameWin == false)
	{
		if (gamePaused == false)
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - boltX, 2) + pow(GetGameObject()->GetPosition().y - boltY, 2) + pow(GetGameObject()->GetPosition().z - boltZ, 2) * 2)) <= 1.0f)
			{
				if (arrowOut == true)
				{

					GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());

				}
			}
			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 4)
			{


				if (GetGameObject()->GetPosition().x > playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x - 0.05, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y > playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y - 0.05, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().x < playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x + 0.05, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y < playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y + 0.05, GetGameObject()->GetPosition().z));
				}

				GetGameObject()->LookAt(glm::vec3(playerX, playerY, 0));



			}

			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 1.5)
			{
				if (playerHealth > 0)
				{
					if (deathTime2 <= 0)
					{
						playerHealth -= 1;
						deathTime2 = 2;
						std::cout << "Player health: " << playerHealth << std::endl;
						canShoot = false;
					}
				}

			}


			if (deathTime2 <= 0)
			{

				canShoot = true;
			}
			if (deathTime2 > 0)
			{
				deathTime2 -= 1 * deltaTime;
			}
		}
	}
}

