#pragma once
#include "Camera.h"

struct Camera2d final
{
	float depth = 1;

	struct Ubo;

	class System final : public CameraSystem<Camera2d, Ubo>
	{
	public:
		explicit System(uint32_t size);

	protected:
		[[nodiscard]] Ubo CreateUbo(Camera2d& camera, uint32_t index) override;
	};

private:
	struct Ubo final
	{
		glm::vec3 position;
		float aspectRatio;
	};
};