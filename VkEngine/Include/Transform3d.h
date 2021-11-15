#pragma once
#include "SoASet.h"

struct Transform3d final
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1};
	bool manualBake = false;

	struct Baked final
	{
		glm::mat4 model{1};
	};

	class System final : public ce::SoASet<Transform3d>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Update();
		void Bake(Transform3d& transform, Baked& bake) const;
	};
};
