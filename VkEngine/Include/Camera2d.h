#pragma once
#include "Camera.h"

struct alignas(4) Camera2d final
{
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