#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class HealthBehaviour1 : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<HealthBehaviour1> Sptr;

	HealthBehaviour1();
	virtual ~HealthBehaviour1();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	std::weak_ptr<Gameplay::IComponent> oneHealth;
public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(HealthBehaviour1);
	virtual nlohmann::json ToJson() const override;
	static HealthBehaviour1::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};