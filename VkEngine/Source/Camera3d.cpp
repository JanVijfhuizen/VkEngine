#include "pch.h"
#include "Camera3d.h"
#include "Transform3d.h"
#include "VkRenderer/WindowSystemGLFW.h"

Camera3d::System::System(const uint32_t size) : CameraSystem<Camera3d, Ubo>(size)
{
}

Camera3d::Ubo Camera3d::System::CreateUbo(Camera3d& camera, const uint32_t index)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& windowSystem = renderSystem.GetWindowSystem();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	auto& transforms = Transform3d::System::Instance::Get();
	auto& transform = transforms[index];

	Ubo ubo{};

	ubo.view = glm::lookAt(transform.position, camera.lookat, glm::vec3(0, 1, 0));
	ubo.projection = glm::perspective(glm::radians(camera.fieldOfView),
		aspectRatio, camera.clipNear, camera.clipFar);

	return ubo;
}
