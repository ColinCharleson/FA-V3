#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class BandageCount : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<BandageCount> Sptr;

	BandageCount();
	virtual ~BandageCount();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	std::weak_ptr<Gameplay::IComponent> BandagePack;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(BandageCount);
	virtual nlohmann::json ToJson() const override;
	static BandageCount::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};