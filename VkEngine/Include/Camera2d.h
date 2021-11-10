#pragma once
#include "Camera.h"

struct Camera2dUbo final
{
	glm::vec3 position;
	float aspectRatio;
};

class Camera2dSystem final : public Camera::System<Camera2dUbo>
{
public:
	explicit Camera2dSystem(uint32_t size);
	Camera2dUbo CreateUbo(Camera& camera, uint32_t index) override;
};

