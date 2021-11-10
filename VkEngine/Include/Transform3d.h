#pragma once
#include "SoASet.h"

struct Transform3d final
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1};

	struct Bake final
	{
		bool manual = false;
		glm::mat4 model;
	};

	class System final : public ce::SoASet<Transform3d>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Update();
	};
};
