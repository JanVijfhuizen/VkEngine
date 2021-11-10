#pragma once
#include "Camera.h"

struct Camera3dUbo final
{
	glm::mat4 view;
	glm::mat4 projection;
};

class Camera3dSystem final : public Camera::System<Camera3dUbo>
{
public:
	glm::vec3 lookat{};
	float fieldOfView = 45;
	float clipNear = 0;
	float clipFar = 1e3f;

protected:
	[[nodiscard]] Camera3dUbo CreateUbo(Camera& camera, uint32_t index) override;
};
