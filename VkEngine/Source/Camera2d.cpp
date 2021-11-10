#include "pch.h"
#include "Camera2d.h"
#include "Transform2d.h"
#include "VkRenderer/WindowSystemGLFW.h"

Camera2dSystem::Camera2dSystem(const uint32_t size): System<Camera2dUbo>(size)
{
}

Camera2dUbo Camera2dSystem::CreateUbo(Camera&, const uint32_t index)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& windowSystem = renderSystem.GetWindowSystem();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	auto& transforms = Transform2d::System::Instance::Get();
	auto& transform = transforms[index];

	Camera2dUbo ubo{};
	ubo.position = glm::vec3(transform.position, 1);
	ubo.aspectRatio = aspectRatio;

	return ubo;
}
