#include "Gameplay/Components/EnemyAi.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
using namespace std;
void EnemyBehaviour::Awake()
{

	_body = GetComponent<Gameplay::Physics::RigidBody>();

	if (_body == nullptr) {
		_playerInTrigger = false;
		_startPosiiton = GetGameObject()->GetPosition();
	}
}


void EnemyBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json EnemyBehaviour::ToJson() const {
	return {
		{ "impulse", _impulse }
	};
}

EnemyBehaviour::EnemyBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }


EnemyBehaviour::Sptr EnemyBehaviour::FromJson(const nlohmann::json& blob) {
	EnemyBehaviour::Sptr result = std::make_shared<EnemyBehaviour>();
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
 int skeletonHealth = 2;
 float dmgTime = 0;


EnemyBehaviour::~EnemyBehaviour() = default;

float deathTime = 0;

void EnemyBehaviour::Update(float deltaTime) 
{
	RenderComponent::Sptr _renderer = GetGameObject()->Get<RenderComponent>();
	if (gameWin == false)
	{
		if (gamePaused == false)
		{
			if ((sqrt(pow(GetGameObject()->GetPosition().x - boltX, 2) + pow(GetGameObject()->GetPosition().y - boltY, 2) + pow((GetGameObject()->GetPosition().z+0.5f) - boltZ, 2) * 2)) <= 1.0f)
			{
				if (arrowOut == true)
				{
					if (dmgTime <= 0)
					{
						skeletonHealth -= 1;
						if (skeletonHealth == 1)
						{
							Texture2D::Sptr oneHealth = ResourceManager::CreateAsset<Texture2D>("textures/SkeletonTex_Skeleton_1Health_1000100010_FXIX.png");
							_renderer->GetMaterial()->Set("u_Material.AlbedoMap", oneHealth);
						}
						dmgTime = 2;

						if (skeletonHealth == 0)
						{
							GetGameObject()->GetScene()->RemoveGameObject(GetGameObject()->SelfRef());
						}
					}
				}
			}


			if (dmgTime >= 0)
			{
				dmgTime -= 1 * deltaTime;
			}

			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 4)
			{


				if (GetGameObject()->GetPosition().x > playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x - 0.02, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y > playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y - 0.02, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().x < playerX)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x + 0.02, GetGameObject()->GetPosition().y, GetGameObject()->GetPosition().z));
				}

				if (GetGameObject()->GetPosition().y < playerY)
				{
					GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y + 0.02, GetGameObject()->GetPosition().z));
				}

				GetGameObject()->LookAt(glm::vec3(playerX, playerY, 0));



			}

			if ((sqrt(pow(GetGameObject()->GetPosition().x - playerX, 2) + pow(GetGameObject()->GetPosition().y - playerY, 2) * 2)) <= 1.5)
			{
				if (playerHealth > 0)
				{
					if (deathTime <= 0)
					{
						playerHealth -= 1;
						deathTime = 2;
						canShoot = false;
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
	}
}

