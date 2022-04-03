#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class HealthBehaviour2 : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<HealthBehaviour2> Sptr;

	HealthBehaviour2();
	virtual ~HealthBehaviour2();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	std::weak_ptr<Gameplay::IComponent> twoHealth;
public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(HealthBehaviour2);
	virtual nlohmann::json ToJson() const override;
	static HealthBehaviour2::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};