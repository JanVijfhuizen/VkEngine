#pragma once
#include "Camera.h"

struct Camera3d final
{
	enum class Type
	{
		Orthographic,
		Perspective
	};

	glm::vec3 lookat{};
	float fieldOfView = 45;
	float clipNear = 0.1f;
	float clipFar = 1e3f;

	struct Ubo;

	class System final : public CameraSystem<Camera3d, Ubo>
	{
	public:
		explicit System(uint32_t size);

	protected:
		[[nodiscard]] Ubo CreateUbo(Camera3d& camera, uint32_t index) override;
	};

private:
	struct Ubo final
	{
		glm::mat4 view;
		glm::mat4 projection;
	};
};

