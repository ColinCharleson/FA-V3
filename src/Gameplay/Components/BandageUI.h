#pragma once
#include "IComponent.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class BandageUI : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<BandageUI> Sptr;

	BandageUI();
	virtual ~BandageUI();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	std::weak_ptr<Gameplay::IComponent> Bandageui;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(BandageUI);
	virtual nlohmann::json ToJson() const override;
	static BandageUI::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _impulse;

	bool _isPressed = false;
};