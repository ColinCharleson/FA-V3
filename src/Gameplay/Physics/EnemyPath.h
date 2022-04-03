 #pragma once

#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"

// Utils
#include "Utils/GUID.hpp"

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/gtx/common.hpp"

// Others
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/ComponentManager.h"

	class EnemyPath : public Gameplay::IComponent
	{
	public:

		typedef std::shared_ptr<EnemyPath> Sptr;
		
		EnemyPath();
		virtual ~EnemyPath();

	

		

		void Update(float deltaTime) ;
	

	public:
		virtual void RenderImGui() override;
		MAKE_TYPENAME(EnemyPath);
		virtual nlohmann::json ToJson() const override;
		static EnemyPath::Sptr FromJson(const nlohmann::json& blob);
		float m_segmentTravelTime = 8.0f;

	
	
	private:
		size_t m_segmentIndex = 0;

	protected:
		float _impulse;
		float _timer = 0.0f;
		
		float _playbackSpeed;
	};

	