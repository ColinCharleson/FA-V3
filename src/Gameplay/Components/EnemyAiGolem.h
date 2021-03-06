#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"


/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class EnemyBehaviourGolem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<EnemyBehaviourGolem> Sptr;

	EnemyBehaviourGolem();
	virtual ~EnemyBehaviourGolem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;



	float StartVelocity;
private:
	float Range;
	float _desiredVelocity;
	float _maxVelocity;
	
	glm::vec3 _movementVector, ArrivalVector, RangeVec;
	glm::vec3 _startPosiiton;
	
public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(EnemyBehaviourGolem);
	virtual nlohmann::json ToJson() const override;
	static EnemyBehaviourGolem::Sptr FromJson(const nlohmann::json& blob);

	std::weak_ptr<Gameplay::GameObject> Player;

protected:
	float _impulse;
	bool _playerInTrigger = false;
	bool _Knockback = false;
	
	Gameplay::Physics::RigidBody::Sptr _body;
};