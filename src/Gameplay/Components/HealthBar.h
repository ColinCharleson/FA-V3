#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class HealthBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<HealthBehaviour> Sptr;

	HealthBehaviour();
	virtual ~HealthBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	std::weak_ptr<Gameplay::IComponent> threeHealth;
public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(HealthBehaviour);
	virtual nlohmann::json ToJson() const override;
	static HealthBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};