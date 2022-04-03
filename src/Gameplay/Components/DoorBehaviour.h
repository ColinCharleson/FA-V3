#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class DoorBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<DoorBehaviour> Sptr;

	DoorBehaviour();
	virtual ~DoorBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(DoorBehaviour);
	virtual nlohmann::json ToJson() const override;
	static DoorBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};