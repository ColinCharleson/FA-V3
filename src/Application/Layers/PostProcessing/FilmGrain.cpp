#include "FilmGrain.h"

#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "../RenderLayer.h"
#include "Application/Application.h"

filmGrain::filmGrain() :
	PostProcessingLayer::Effect(), 
	_shader(nullptr)
{
	Name = "Film Grain";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/Film_Grain.glsl" }
	});
}

filmGrain::~filmGrain() = default;

void filmGrain::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
	gBuffer->BindAttachment(RenderTargetAttachment::Depth, 1);
}

void filmGrain::RenderImGui()
{
	const auto& cam = Application::Get().CurrentScene()->MainCamera;

	if (cam != nullptr) {
		ImGui::DragFloat("Focal Depth", &cam->FocalDepth, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Lens Dist. ", &cam->LensDepth,  0.01f, 0.001f, 50.0f);
		ImGui::DragFloat("Aperture   ", &cam->Aperture,   0.1f, 0.1f, 60.0f);
	}
}

filmGrain::Sptr filmGrain::FromJson(const nlohmann::json& data)
{
	filmGrain::Sptr result = std::make_shared<filmGrain>();
	result->Enabled = JsonGet(data, "enabled", true);
	return result;
}

nlohmann::json filmGrain::ToJson() const
{
	return {
		{ "enabled", Enabled }
	};
}
