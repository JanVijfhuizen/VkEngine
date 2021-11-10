#include "pch.h"
#include "Camera3d.h"
#include "Transform3d.h"
#include "glm/ext.hpp"
#include "VkRenderer/WindowSystemGLFW.h"

Camera3dUbo Camera3dSystem::CreateUbo(Camera&, const uint32_t index)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& windowSystem = renderSystem.GetWindowSystem();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	auto& transforms = Transform3d::System::Instance::Get();
	auto& transform = transforms[index];

	Camera3dUbo ubo{};

	ubo.view = glm::lookAt(transform.position, lookat, glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::vec3(0.0f, 1.0f, 0.0f);
	ubo.projection = glm::perspective(glm::radians(fieldOfView),
		aspectRatio, clipNear, clipFar);

	return ubo;
}
