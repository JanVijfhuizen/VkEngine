#include "pch.h"
#include "Camera2d.h"
#include "Transform2d.h"
#include "VkRenderer/WindowSystemGLFW.h"

Camera2d::System::System(const uint32_t size) : CameraSystem<Camera2d, Ubo>(size)
{
}

Camera2d::Ubo Camera2d::System::CreateUbo(Camera2d&, const uint32_t index)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& windowSystem = renderSystem.GetWindowSystem();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	auto& transforms = Transform2d::System::Instance::Get();
	auto& transform = transforms[index];

	Ubo ubo{};
	ubo.position = glm::vec3(transform.position, 1);
	ubo.aspectRatio = aspectRatio;

	return ubo;
}
